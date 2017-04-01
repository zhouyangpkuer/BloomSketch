#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>

#include "CMSketch.h"
#include "ASketch.h"
#include "CUSketch.h"
#include "CSketch.h"
#include "BloomSketch.h"
#include "g_BloomSketch.h"

// #define layer2
// #define layer3
#define layer4


using namespace std;

char * filename_FlowTraffic = "../2";

char insert[10000000 + 10000000 / 5][200];

unordered_map<string, int> unmp;


int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

	unmp.clear();
	uint ip_s, ip_d;
	int val;

    int package_num = 0;
    FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");
    while(fgets(insert[package_num], 200, file_FlowTraffic) != NULL)
    {
        unmp[string(insert[package_num])]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);

    printf("dataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());
    
    int max_freq = 0;
    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);




    double memory = 1.0;

    FILE * file_coef = fopen("./coef_res.txt", "w");

    double coef_low = 0.7;
    double coef_bf = 0.10;


    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;    
    int w_l = coef_low * memory * 1024 * 1024 * 8.0 / LOW_COUNTER_SIZE;
    int w_bf = coef_bf * memory * 1024 * 1024 * 8.0 / 1;
    int w_h = (1 - coef_low - coef_bf) * memory * 1024 * 1024 * 8.0 / HIGH_COUNTER_SIZE;
    int w_bs = memory * 1024 * 1024 * 8.0 / 64;


    printf("w_l = %d, w_h = %d\n", w_l, w_h);


    
    CMSketch *cmsketch;
    CUSketch *cusketch;
    CSketch *csketch;
    ASketch *asketch;
    BloomSketch *bs;
    g_BloomSketch *gbs;



    
    cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    csketch = new CSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    asketch = new ASketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    bs = new BloomSketch(w_l, w_h, 4, 4, w_bf, 4);
    


#ifdef layer2
    
    int number_layer = 2;
    int W[2] = {w_l, w_h};
    int D[2] = {4, 4};
    int W_BF[1] = {w_bf};
    int SIZE_COUNTER[2] = {4, 12};

#endif



#ifdef layer3

    double s = 0.90;
    double u = 8;

    double all_u = 1 + 1.0 / u + 1.0 / (u * u);
    double mem_1 = memory * s * 1.0 / all_u;
    double mem_2 = memory * s * 1.0 * (1.0 / u) / all_u;
    double mem_3 = memory * s * 1.0 * (1.0 / (u * u)) / all_u;


    double all_u_bf = 1 + 1.0 / u;
    double mem_1_bf = memory * (1 - s) * 1.0 / all_u_bf;
    double mem_2_bf = memory * (1 - s) * 1.0 * (1.0 / u) / all_u_bf;



    int number_layer = 3;    
    int W[] = {mem_1 * 1024 * 1024 * 8.0 / 4, mem_2 * 1024 * 1024 * 8.0 / 4, mem_3 * 1024 * 1024 * 8.0 / 8};
    int D[] = {4, 4, 4};
    int W_BF[] = {mem_1_bf * 1024 * 1024 * 8, mem_2_bf * 1024 * 1024 * 8};
    int SIZE_COUNTER[] = {4, 4, 8};

#endif




#ifdef layer4

    double t = 0.90;
    double v = 8;

    double w_all = memory * t * 1024 * 1024 * 8.0 / 4;
    double all = 1 + 1.0 / v + 1.0 / (v * v) + 1.0 / (v * v * v);

    int w_1 = w_all * 1 / all;
    int w_2 = w_all * (1 / all) / v;
    int w_3 = w_all * (1 / all) / (v * v);
    int w_4 = w_all * (1 / all) / (v * v * v);




    double w_all_bf = memory * (1 - t) * 1024 * 1024 * 8.0 / 1;
    double all_bf = 1 + 1.0 / v + 1.0 / (v * v);

    int w_1_bf = w_all_bf * 1 / all_bf;
    int w_2_bf = w_all_bf * (1 / all_bf) / v;
    int w_3_bf = w_all_bf * (1 / all_bf) / (v * v);



    int number_layer = 4;
    int W[] = {w_1, w_2, w_3, w_4};
    int D[] = {4, 4, 4, 4};
    // int D[] = {3, 3, 3, 3};
    // int D[] = {2, 2, 2, 2};

    int W_BF[] = {w_1_bf, w_2_bf, w_3_bf};
    int SIZE_COUNTER[] = {4, 4, 4, 4};

#endif


    gbs = new g_BloomSketch(number_layer, W, D, W_BF, SIZE_COUNTER);

    
    for(int i = 0; i < package_num; i++)
    {
        cmsketch->Insert(insert[i]);
        cusketch->Insert(insert[i]);
        csketch->Insert(insert[i]);
        asketch->Insert(insert[i]);
        bs->Insert(insert[i]);
        gbs->Insert(insert[i]);
    }

    

    char temp[500];

    double re_cm = 0.0, re_cu = 0.0, re_c = 0.0, re_a = 0.0, re_bs = 0.0, re_bcm = 0.0, re_gbs = 0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0, re_c_sum = 0.0, re_a_sum = 0.0, re_bs_sum = 0.0, re_bcm_sum = 0.0, re_gbs_sum = 0.0;

    double ae_cm = 0.0, ae_cu = 0.0, ae_c = 0.0, ae_a = 0.0, ae_bs = 0.0, ae_bcm = 0.0, ae_gbs = 0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0, ae_c_sum = 0.0, ae_a_sum = 0.0, ae_bs_sum = 0.0, ae_bcm_sum = 0.0, ae_gbs_sum = 0.0;

	int val_cm = 0, val_cu = 0, val_c = 0, val_a = 0, val_bs = 0, val_bcm = 0, val_gbs = 0;
    
    int max_val = 0;






    FILE * file_res = fopen("./res.txt", "w");

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
    	strcpy(temp, (it->first).c_str());
    	val = it->second;
		
        max_val = max_val > val ? max_val : val;
        

		val_cm = cmsketch->Query(temp);
        val_cu = cusketch->Query(temp);
        val_c = csketch->Query(temp);
		val_a = asketch->Query(temp);
        val_bs = bs->Query(temp);
        val_gbs = gbs->Query(temp);


 	    

        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_c = fabs(val_c - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
        re_bs = fabs(val_bs - val) / (val * 1.0);
        re_gbs = fabs(val_gbs - val) / (val * 1.0);


        
        ae_cm = fabs(val_cm - val);
        ae_cu = fabs(val_cu - val);
        ae_c = fabs(val_c - val);
        ae_a = fabs(val_a - val);
        ae_bs = fabs(val_bs - val);
        ae_gbs = fabs(val_gbs - val);


        re_cm_sum += re_cm;
        re_cu_sum += re_cu;
        re_c_sum += re_c;
        re_a_sum += re_a;
        re_bs_sum += re_bs;
        re_gbs_sum += re_gbs;


        ae_cm_sum += ae_cm;
        ae_cu_sum += ae_cu;
        ae_c_sum += ae_c;
        ae_a_sum += ae_a;
        ae_bs_sum += ae_bs;
        ae_gbs_sum += ae_gbs;



        fprintf(file_res, "%d, %d, %d, %d, %d, %d, %d\n", val, val_cm, val_cu, val_c, val_a, val_bs, val_gbs);
    }
    fclose(file_res);








    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;


    fprintf(stderr, "re_cm %lf\n", re_cm_sum / b);
    fprintf(stderr, "re_cu %lf\n", re_cu_sum / b);
    fprintf(stderr, "re_c %lf\n", re_c_sum / b);
    fprintf(stderr, "re_a %lf\n", re_a_sum / b);
    fprintf(stderr, "re_bs %lf\n", re_bs_sum / b);
    fprintf(stderr, "re_gbs %lf\n", re_gbs_sum / b);


    
    fprintf(stderr, "\nae_cm %lf\n", ae_cm_sum / b);
    fprintf(stderr, "ae_cu %lf\n", ae_cu_sum / b);
    fprintf(stderr, "ae_c %lf\n", ae_c_sum / b);
    fprintf(stderr, "ae_a %lf\n", ae_a_sum / b);
    fprintf(stderr, "ae_bs %lf\n", ae_bs_sum / b);
    fprintf(stderr, "ae_gbs %lf\n", ae_gbs_sum / b);
    
    printf("\nman_insert %lf\n", bs->man_insert * 1.0 / a);
    printf("man_query %lf\n", bs->man_query * 1.0 / b);

    printf("\ng_man_insert %lf\n", gbs->man_insert * 1.0 / a);
    printf("g_man_query %lf\n", gbs->man_query * 1.0 / b);


    fprintf(file_coef, "%lf\t%lf\t%lf\t%lf\n", coef_low, coef_bf, re_bs_sum / b, ae_bs_sum / b);

    return 0;

}

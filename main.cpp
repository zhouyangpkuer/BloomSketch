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
#include "CUSketch.h"
#include "CSketch.h"
#include "g_BloomSketch_omabf_reuse.h"

using namespace std;

// #define layer2
// #define layer3
#define layer4


char * filename_FlowTraffic = "stream.dat";

char insert[10000000 + 10000000 / 5][200];

/* for query */
char **query;

unordered_map<string, int> unmp;


#define SPEED_TEST

#define testcycles 10


int main(int argc, char** argv)
{
    if(argc == 2)
        filename_FlowTraffic = argv[1];
   
	unmp.clear();
	int val;


/******************************* read in data ********************************/

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


    printf("\ndataset: %s\n", filename_FlowTraffic);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());



    query = new char*[unmp.size()];
    for(int i = 0; i < unmp.size(); ++i)
        query[i] = new char[200];

    int distinctFlowNum = unmp.size();

    
    int max_freq = 0;

    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        strcpy(query[i], (it->first).c_str());

        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);



    printf("\n*************************************\n\n");





/******************************* memory size ********************************/


    double memory = 0.1;


    double coef_low = 0.75;
    double coef_bf = 0.12;



    printf("The memory size is %lf MB\n", memory);

    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;    
    int w_l = coef_low * memory * 1024 * 1024 * 8.0 / LOW_COUNTER_SIZE;
    int w_bf = coef_bf * memory * 1024 * 1024 * 8.0 / 1;
    int w_h = (1 - coef_low - coef_bf) * memory * 1024 * 1024 * 8.0 / HIGH_COUNTER_SIZE;
    int w_bs = memory * 1024 * 1024 * 8.0 / 64;

    printf("\nw_l = %d, w_h = %d\n", w_l, w_h);

    printf("\n*************************************\n\n");


#ifdef layer2
    
    int number_layer = 2;
    int W[2] = {w_l, w_h};
    int D[2] = {4, 4};
    int W_BF[1] = {w_bf};
    int SIZE_COUNTER[2] = {4, 12};

#endif



#ifdef layer3

    double s = 0.90;        // s 表示sketch占比
    double u = 8;           // u 表示比例

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
    int W_BF[] = {w_1_bf, w_2_bf, w_3_bf};
    int SIZE_COUNTER[] = {4, 4, 4, 4};

#endif


    CMSketch *cmsketch;
    CUSketch *cusketch;
    CSketch *csketch;
    g_BloomSketch *gbs;



/******************************* throughput test ********************************/

#ifdef SPEED_TEST



    timespec time1, time2;
    long long resns;


    double throughput_cm_insert, throughput_cu_insert, throughput_c_insert, throughput_gbs_insert;

    double throughput_cm_query, throughput_cu_query, throughput_c_query, throughput_gbs_query;

/* insert throughput */

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; ++i)
        {
            cmsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm_insert = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CM (insert): %.6lf Mips\n", throughput_cm_insert);




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; ++i)
        {
            cusketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cu_insert = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of CU (insert): %.6lf Mips\n", throughput_cu_insert);
    



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
       csketch = new CSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; ++i)
        {
            csketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_c_insert = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of C (insert): %.6lf Mips\n", throughput_c_insert);
    



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        gbs = new g_BloomSketch(number_layer, W, D, W_BF, SIZE_COUNTER);
        for(int i = 0; i < package_num; ++i)
        {
            gbs->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_gbs_insert = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of bloomsketch (insert): %.6lf Mips\n", throughput_gbs_insert);




    /* query throughput */

/* avoid the over-optimize of the compiler! */
int aaaa;

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        for(int i = 0; i < distinctFlowNum; ++i)
        {
            aaaa = cmsketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm_query = (double)1000.0 * testcycles * distinctFlowNum / resns;
    printf("\n\nthroughput of CM (query): %.6lf Mips\n", throughput_cm_query);
    



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        for(int i = 0; i < distinctFlowNum; ++i)
        {
            aaaa = cusketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cu_query = (double)1000.0 * testcycles * distinctFlowNum / resns;
    printf("throughput of CU (query): %.6lf Mips\n", throughput_cu_query);
    



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        for(int i = 0; i < distinctFlowNum; ++i)
        {
            aaaa = csketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_c_query = (double)1000.0 * testcycles * distinctFlowNum / resns;
    printf("throughput of C (query): %.6lf Mips\n", throughput_c_query);




    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; ++t)
    {
        for(int i = 0; i < distinctFlowNum; ++i)
        {
            aaaa = gbs->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_gbs_query = (double)1000.0 * testcycles * distinctFlowNum / resns;
    printf("throughput of bloomsketch (query): %.6lf Mips\n", throughput_gbs_query);



    /* avoid the over-optimize of the compiler! */
    if(aaaa == (1 <<30))
        printf("______________\n");

    printf("\n*************************************\n\n");

#endif




/***************************** accuracy test ********************************/ 


    cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    csketch = new CSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    gbs = new g_BloomSketch(number_layer, W, D, W_BF, SIZE_COUNTER);

    
    for(int i = 0; i < package_num; i++)
    {
        cmsketch->Insert(insert[i]);
        cusketch->Insert(insert[i]);
        csketch->Insert(insert[i]);
        gbs->Insert(insert[i]);
    }



    double re_cm = 0.0, re_cu = 0.0, re_c = 0.0, re_a = 0.0, re_bs = 0.0, re_bcm = 0.0, re_gbs = 0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0, re_c_sum = 0.0, re_a_sum = 0.0, re_bs_sum = 0.0, re_bcm_sum = 0.0, re_gbs_sum = 0.0;

    double ae_cm = 0.0, ae_cu = 0.0, ae_c = 0.0, ae_a = 0.0, ae_bs = 0.0, ae_bcm = 0.0, ae_gbs = 0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0, ae_c_sum = 0.0, ae_a_sum = 0.0, ae_bs_sum = 0.0, ae_bcm_sum = 0.0, ae_gbs_sum = 0.0;

	int val_cm = 0, val_cu = 0, val_c = 0, val_a = 0, val_bs = 0, val_bcm = 0, val_gbs = 0;




    int gbs_cor_num = 0, cm_cor_num = 0, cu_cor_num = 0, c_cor_num = 0;

    char temp[500];

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
    	strcpy(temp, (it->first).c_str());
    	val = it->second;

        

		val_cm = cmsketch->Query(temp);
        val_cu = cusketch->Query(temp);
        val_c = csketch->Query(temp);
        val_gbs = gbs->Query(temp);


        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_c = fabs(val_c - val) / (val * 1.0);
        re_gbs = fabs(val_gbs - val) / (val * 1.0);

        
        ae_cm = fabs(val_cm - val);
        ae_cu = fabs(val_cu - val);
        ae_c = fabs(val_c - val);
        ae_gbs = fabs(val_gbs - val);


        re_cm_sum += re_cm;
        re_cu_sum += re_cu;
        re_c_sum += re_c;
        re_gbs_sum += re_gbs;


        ae_cm_sum += ae_cm;
        ae_cu_sum += ae_cu;
        ae_c_sum += ae_c;
        ae_gbs_sum += ae_gbs;


        if(val == val_gbs)
            gbs_cor_num++;
        if(val == val_cm)
            cm_cor_num++;
        if(val == val_cu)
            cu_cor_num++;
        if(val == val_c)
            c_cor_num++;
    }



    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;


    /*  *************************************** */
    printf("correct rate of CM = %lf\n", cm_cor_num * 1.0 / b);
    printf("correct rate of CU = %lf\n", cu_cor_num * 1.0 / b);
    printf("correct rate of C = %lf\n", c_cor_num * 1.0 / b);
    printf("correct rate of bloomsketch = %lf\n\n", gbs_cor_num * 1.0 / b);

    

    /*  *************************************** */
    printf("are_cm %lf\n", re_cm_sum / b);
    printf("are_cu %lf\n", re_cu_sum / b);
    printf("are_c %lf\n", re_c_sum / b);
    printf("are_gbs %lf\n\n", re_gbs_sum / b);


    
    /*  *************************************** */   
    printf("aae_cm %lf\n", ae_cm_sum / b);
    printf("aae_cu %lf\n", ae_cu_sum / b);
    printf("aae_c %lf\n", ae_c_sum / b);
    printf("aae_gbs %lf\n\n", ae_gbs_sum / b);

    

    /*  *************************************** */  
    printf("\n*************************************\n\n");
    printf("CM man_insert %lf\n", cmsketch->mem_acc_ins * 1.0 / a);
    printf("CM man_query %lf\n\n", cmsketch->mem_acc_query * 1.0 / b);
    printf("CU man_insert %lf\n", cusketch->mem_acc_ins * 1.0 / a);
    printf("CU man_query %lf\n\n", cusketch->mem_acc_query * 1.0 / b);
    printf("C man_insert %lf\n", csketch->mem_acc_ins * 1.0 / a);
    printf("C man_query %lf\n\n", csketch->mem_acc_query * 1.0 / b);
    printf("bloomsketch man_insert %lf\n", gbs->man_insert * 1.0 / a);
    printf("bloomsketch man_query %lf\n\n", gbs->man_query * 1.0 / b);
    

    return 0;
}



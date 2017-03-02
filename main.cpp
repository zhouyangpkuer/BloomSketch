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
#include "BCMSketch.h"


using namespace std;

// char * filename_FlowTraffic = "../real_flow_insert.txt";

// char * filename_FlowTraffic = "../kosarak.dat";
// char * filename_FlowTraffic = "../retail.dat";
// char * filename_FlowTraffic = "../accidents.dat";

// char * filename_FlowTraffic = "../webdocs.dat";
// char * filename_FlowTraffic = "1";
// char * filename_FlowTraffic = "../PyramidCounting-Experiment-speed/zipf_0_insert.txt";
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


    double memory = 1.0;


    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;
    int w_bs = memory * 1024 * 1024 * 8.0 / 64;

    
    CMSketch *cmsketch;
    CUSketch *cusketch;
    CSketch *csketch;
    ASketch *asketch;
    BloomSketch *bs;
    BCMSketch *bcm;


    
    cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    csketch = new CSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    asketch = new ASketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
    bs = new BloomSketch(w_bs * 13, w_bs, 3, 4);
    bcm = new BCMSketch(w_bs * 13, w_bs, 1, 4);



    char _temp[200];
    int package_num = 0;
 	FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");
    // while(fscanf(file_FlowTraffic, "%u %u %d", &ip_s, &ip_d, &val) != EOF)
    // while(fscanf(file_FlowTraffic, "%s", insert[0]) != EOF)
    while(fgets(insert[package_num], 200, file_FlowTraffic) != NULL)
    {
        insert[0][strlen(insert[package_num]) - 1] = '\0';

        // printf("%s\n", insert[0]);


        // strcpy(_temp, insert[0]);
        // strcat(insert[0], _temp);

        // sprintf(insert[0], "%u%u", ip_s, ip_d);
        unmp[string(insert[package_num])]++;

        // printf("%s\n", insert[0]);
        // fscanf(file_FlowTraffic, "%d", &ip_s);
        // sprintf(insert[0], "%d", ip_s);
        // unmp[string(insert[0])]++;

        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_FlowTraffic);

    for(int i = 0; i < package_num; i++)
    {
        cmsketch->Insert(insert[i]);
        cusketch->Insert(insert[i]);
        csketch->Insert(insert[i]);
        asketch->Insert(insert[i]);
        bs->Insert(insert[i]);
        bcm->Insert(insert[i]);
    }

    
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





    char temp[500];

    double re_cm = 0.0, re_cu = 0.0, re_c = 0.0, re_a = 0.0, re_bs = 0.0, re_bcm = 0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0, re_c_sum = 0.0, re_a_sum = 0.0, re_bs_sum = 0.0, re_bcm_sum = 0.0;

    double ae_cm = 0.0, ae_cu = 0.0, ae_c = 0.0, ae_a = 0.0, ae_bs = 0.0, ae_bcm = 0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0, ae_c_sum = 0.0, ae_a_sum = 0.0, ae_bs_sum = 0.0, ae_bcm_sum = 0.0;

	int val_cm = 0, val_cu = 0, val_c = 0, val_a = 0, val_bs = 0, val_bcm = 0;
    
    int max_val = 0;


    int bucket_num[11110];
    double cm_bucket_re[11110];
    double cu_bucket_re[11110];
    double c_bucket_re[11110];
    double a_bucket_re[11110];
    double bs_bucket_re[11110];




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
        val_bcm = bcm->Query(temp);

 	    

        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_c = fabs(val_c - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
        re_bs = fabs(val_bs - val) / (val * 1.0);
        re_bcm = fabs(val_bcm - val) / (val * 1.0);

        
        ae_cm = fabs(val_cm - val);
        ae_cu = fabs(val_cu - val);
        ae_c = fabs(val_c - val);
        ae_a = fabs(val_a - val);
        ae_bs = fabs(val_bs - val);
        ae_bcm = fabs(val_bcm - val);


        re_cm_sum += re_cm;
        re_cu_sum += re_cu;
        re_c_sum += re_c;
        re_a_sum += re_a;
        re_bs_sum += re_bs;
        re_bcm_sum += re_bcm;


        ae_cm_sum += ae_cm;
        ae_cu_sum += ae_cu;
        ae_c_sum += ae_c;
        ae_a_sum += ae_a;
        ae_bs_sum += ae_bs;
        ae_bcm_sum += ae_bcm;



        if(val <= 10000)
        {
            bucket_num[val] ++;
            cm_bucket_re[val] += re_cm;
            cu_bucket_re[val] += re_cu;
            c_bucket_re[val] += re_c;
            a_bucket_re[val] += re_a;
            bs_bucket_re[val] += re_bs;

        }

        fprintf(file_res, "%d, %d, %d, %d, %d, %d\n", val, val_cm, val_cu, val_c, val_a, val_bs, val_bcm);
    }
    fclose(file_res);





    double pre_cm = 0;
    double pre_cu = 0;
    double pre_c = 0;
    double pre_a = 0;
    double pre_bs = 0;



    FILE* file_output = fopen("bucket_figure","w");
    fprintf(file_output,"Figure 10,CM,CU,C,A,BS\n");

    for(int i = 1;i <= 10000; i++)
    {
        double cm_dot,cu_dot,c_dot,a_dot,bs_dot;
        if(bucket_num[i]==0)
        {
            continue;
        }
        else
        {
            if(cm_bucket_re[i]==0){
                cm_dot=pre_cm;
            }
            else{
                cm_dot=pre_cm=cm_bucket_re[i]/bucket_num[i];
            }
            if(cu_bucket_re[i]==0){
                cu_dot=pre_cu;
            }
            else{
                cu_dot=pre_cu=cu_bucket_re[i]/bucket_num[i];
            }
            if(c_bucket_re[i]==0){
                c_dot=pre_c;
            }
            else{
                c_dot=pre_c=c_bucket_re[i]/bucket_num[i];
            }
            if(a_bucket_re[i]==0){
                a_dot=pre_a;
            }
            else{
                a_dot=pre_a=a_bucket_re[i]/bucket_num[i];
            }
            if(bs_bucket_re[i]==0){
                bs_dot=pre_bs;
            }
            else{
                bs_dot=pre_bs=bs_bucket_re[i]/bucket_num[i];
            }
        }
        fprintf(file_output,"%lf,%lf,%lf,%lf,%lf,%lf\n",log10(i),
            cm_dot,cu_dot,c_dot,a_dot,bs_dot);
    }







    double a = package_num * 1.0;
    double b = unmp.size() * 1.0;


    fprintf(stderr, "re_cm %lf\n", re_cm_sum / b);
    fprintf(stderr, "re_cu %lf\n", re_cu_sum / b);
    fprintf(stderr, "re_c %lf\n", re_c_sum / b);
    fprintf(stderr, "re_a %lf\n", re_a_sum / b);
    fprintf(stderr, "re_bs %lf\n", re_bs_sum / b);
    fprintf(stderr, "re_bcm %lf\n", re_bcm_sum / b);

    
    fprintf(stderr, "\nae_cm %lf\n", ae_cm_sum / b);
    fprintf(stderr, "ae_cu %lf\n", ae_cu_sum / b);
    fprintf(stderr, "ae_c %lf\n", ae_c_sum / b);
    fprintf(stderr, "ae_a %lf\n", ae_a_sum / b);
    fprintf(stderr, "ae_bs %lf\n", ae_bs_sum / b);
    fprintf(stderr, "ae_bcm %lf\n", ae_bcm_sum / b);
    
    return 0;

}

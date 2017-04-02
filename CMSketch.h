#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash32.h"
#include <iostream>

using namespace std;

class CMSketch
{	
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	uint16_t *counter[MAX_HASH_NUM];
	int w, d;
	int MAX_CNT;
	int counter_index_size;
	uint64_t hash_value;

public:
	int mem_acc_ins, mem_acc_query;
	CMSketch(int _w, int _d)
	{
		mem_acc_ins = 0;
		mem_acc_query = 0;
		counter_index_size = 20;
		w = _w;
		d = _d;
		
		for(int i = 0; i < d; i++)	
		{
			counter[i] = new uint16_t[w];
			memset(counter[i], 0, sizeof(uint16_t) * w);
		}

		MAX_CNT = (1 << COUNTER_SIZE) - 1;

		for(int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(const char * str)
	{
		// for(int i = 0; i < d / 2; i ++)
		// {
		// 	hash_value = bobhash[i]->run(str, strlen(str));
		// 	// printf("%llu\n", hash_value);
			
		// 	index[i] = (hash_value & ((1 << counter_index_size) - 1)) % w;
		// 	hash_value >>= counter_index_size;
		// 	index[i + 1] = (hash_value & ((1 << counter_index_size) - 1)) % w;

		// 	i++;
		// }
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			// if(counter[i][index[i]] != MAX_CNT)
			// {
			counter[i][index[i]]++;
			// }
			mem_acc_ins ++;

		}
	}
	int Query(const char *str)
	{
		int min_value = MAX_CNT;
		int temp;
		// for(int i = 0; i < d / 2; i ++)
		// {
		// 	hash_value = bobhash[i]->run(str, strlen(str));
		// 	index[i] = (hash_value & ((1 << counter_index_size) - 1)) % w;
		// 	hash_value >>= counter_index_size;
		// 	index[i + 1] = (hash_value & ((1 << counter_index_size) - 1)) % w;
			
		// 	i++;
		// }
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
			mem_acc_query ++;
		}
		return min_value;
	}
	~CMSketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}


		for(int i = 0; i < d; i++)
		{
			delete bobhash[i];
		}
	}
};
#endif//_CMSKETCH_H
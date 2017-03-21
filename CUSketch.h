#ifndef _CUSketch_H
#define _CUSketch_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash32.h"
#include <iostream>

using namespace std;

class CUSketch
{	
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	int *counter[MAX_HASH_NUM];
	int w, d;
	int MAX_CNT;
	int counter_index_size;
	uint64_t hash_value;

public:
	int mem_acc_ins, mem_acc_query;
	CUSketch(int _w, int _d)
	{
		mem_acc_ins = 0;
		mem_acc_query = 0;
		counter_index_size = 20;
		w = _w;
		d = _d;
		
		for(int i = 0; i < d; i++)	
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		MAX_CNT = (1 << COUNTER_SIZE) - 1;

		for(int i = 0; i < d; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(const char * str)
	{
		int min_value = 1 << 30;
		int temp;

		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;

			mem_acc_ins++;
		}
		if(min_value == MAX_CNT)
			return;

		for(int i = 0; i < d; i++)
		{
			if(counter[i][index[i]] == min_value)
				counter[i][index[i]] ++;
		}
	}
	int Query(const char *str)
	{
		int min_value = 1 << 30;
		int temp;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
			mem_acc_query++;
		}
		return min_value;
	}
	~CUSketch()
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
#endif//_CUSketch_H
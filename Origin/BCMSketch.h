#ifndef _BCMSketch_H
#define _BCMSketch_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash.h"
#include <iostream>
#include <string>
#include "params.h"
#include <map>


using namespace std;
// int cnt = 0;

class BCMSketch
{	
private:
	BOBHash * bobhash[MAX_HASH_NUM];
	uint64 index_low[MAX_HASH_NUM];
	uint64 index_high[MAX_HASH_NUM];
	
	int *counter_low;
	int *carry_bit;
	int *counter_high;

	int w_low, d_low;
	int w_high, d_high;

	int MAX_CNT_LOW;
	int MAX_CNT_HIGH;
	// map<string, int> mp;

public:
	BCMSketch(int _w_low, int _w_high, int _d_low, int _d_high)
	{
		w_low = _w_low;
		w_high = _w_high;
		d_low = _d_low;
		d_high = _d_high;

		counter_low = new int[w_low];
		carry_bit = new int[w_low];
		counter_high = new int[w_high];

		memset(counter_low, 0, sizeof(int) * w_low);
		memset(carry_bit, 0, sizeof(int) * w_low);
		memset(counter_high, 0, sizeof(int) * w_high);

		MAX_CNT_LOW = (1 << LOW_COUNTER_SIZE) - 1;
		MAX_CNT_HIGH = (1 << HIGH_COUNTER_SIZE) - 1;

		for(int i = 0; i < d_low + d_high; i++)
		{
			bobhash[i] = new BOBHash(i + 1000);
		}
	}
	void Insert(const char * str)
	{
		int temp;
		
		for(int i = 0; i < d_low; i++)
		{
			index_low[i] = (bobhash[i]->run(str, strlen(str)));
			index_high[i] = index_low[i] % w_high;
			index_low[i] %= w_low;

			temp = counter_low[index_low[i]];


			if(temp == MAX_CNT_LOW)
			{
				for(int i = 0; i < d_low; i++)
				{
					carry_bit[index_low[i]] = 1;
					counter_low[index_low[i]] = 0;
				}
				//carry into the high counter!
				for(int i = 0; i < d_high; i++)
				{
					counter_high[index_high[i]]++;
				}
			}
			else
			{
				counter_high[index_low[i]] ++;
			}
		}
	}
	int Query(const char *str)
	{
		int min_value = 1 << 30;
		int temp;
		int flag = 1;

		for(int i = 0; i < d_low; i++)
		{
			index_low[i] = (bobhash[i]->run(str, strlen(str)));
			index_high[i] = index_low[i] % w_high;
			index_low[i] %= w_low;

			if(carry_bit[index_low[i]] == 0)
				flag = 0;

			temp = counter_low[index_low[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		if(flag == 0)
			return min_value;

		int backup = min_value;

		min_value = 1 << 30;
		for(int i = 0; i < d_high; i++)
		{
			temp = counter_high[index_high[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value + min_value * (MAX_CNT_LOW + 1);
	}
	~BCMSketch();
};
#endif//_BCMSketch_H
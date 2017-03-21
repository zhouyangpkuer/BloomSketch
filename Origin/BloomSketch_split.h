#ifndef _BloomSketch_SPLIT_H
#define _BloomSketch_SPLIT_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash32.h"
#include <iostream>
#include <string>
#include "params.h"
#include <map>


using namespace std;
// int cnt = 0;

class BloomSketch
{	
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	uint64 index_low[MAX_HASH_NUM];
	uint64 index_high[MAX_HASH_NUM];
	uint64 index_bf[MAX_HASH_NUM];

	
	int *counter_low;
	int *carry_bit;
	int *counter_high;
	int *bf;

	int w_low, d_low;
	int w_high, d_high;
	int w_bf, d_bf;

	int MAX_CNT_LOW;
	int MAX_CNT_HIGH;
	// map<string, int> mp;

public:
	BloomSketch(int _w_low, int _w_high, int _d_low, int _d_high, int _w_bf, int _d_bf)
	{
		w_low = _w_low;
		w_high = _w_high;
		d_low = _d_low;
		d_high = _d_high;
		w_bf = _w_bf;
		d_bf = _d_bf;

		counter_low = new int[w_low];
		carry_bit = new int[w_low];
		counter_high = new int[w_high];
		bf = new int[w_bf];

		memset(counter_low, 0, sizeof(int) * w_low);
		memset(carry_bit, 0, sizeof(int) * w_low);
		memset(counter_high, 0, sizeof(int) * w_high);
		memset(bf, 0, sizeof(int) * w_bf);

		MAX_CNT_LOW = (1 << LOW_COUNTER_SIZE) - 1;
		MAX_CNT_HIGH = (1 << HIGH_COUNTER_SIZE) - 1;

		for(int i = 0; i < d_low + d_high + d_bf; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
	}
	void Insert(const char * str)
	{
		int min_value = 1 << 30;
		int temp;
		
		for(int i = 0; i < d_low; i++)
		{
			index_low[i] = (bobhash[i]->run(str, strlen(str)));
			index_low[i] %= w_low;

			temp = counter_low[index_low[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		int backup = min_value;

		if(min_value == MAX_CNT_LOW)
		{
			for(int i = 0; i < d_high; i++)
			{
				index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
			}

			// cnt++;
			for(int i = 0; i < d_low; i++)
			{
				counter_low[index_low[i]] = 0;
			}
			for(int i = 0; i < d_bf; i++)
			{
				index_bf[i] = (bobhash[i + d_low + d_high]->run(str, strlen(str))) % w_bf;
				bf[index_bf[i]] = 1;
			}
			
			//carry into the high counter!
			min_value = 1 << 30;
			for(int i = 0; i < d_high; i++)
			{
				temp = counter_high[index_high[i]];
				min_value = temp < min_value ? temp : min_value;
			}
			
			if(min_value != MAX_CNT_HIGH)
			{
				for(int i = 0; i < d_high; i++)
				{
					if(counter_high[index_high[i]] == min_value)
						counter_high[index_high[i]]++;
				}
			}

			return;
		}
		for(int i = 0; i < d_low; i++)
		{
			if(counter_low[index_low[i]] == backup)
				counter_low[index_low[i]]++;
		}
	}
	int Query(const char *str)
	{
		int min_value = 1 << 30;
		int temp;
		int flag = 1;

		for(int i = 0; i < d_low; i++)
		{
			// index[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			index_low[i] = (bobhash[i]->run(str, strlen(str)));
			// index_high[i] = index_low[i] % w_high;
			index_low[i] %= w_low;
			
			temp = counter_low[index_low[i]];
			min_value = temp < min_value ? temp : min_value;
		}

		for(int i = 0; i < d_bf; i++)
		{
			index_bf[i] = (bobhash[i + d_low + d_high]->run(str, strlen(str))) % w_bf;

			if(bf[index_bf[i]] == 0)
			{
				flag = 0;
				break;
			}
		}
		if(flag == 0)
			return min_value;

		int backup = min_value;
		
		for(int i = 0; i < d_high; i++)
		{
			index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
		}
		
		min_value = 1 << 30;
		for(int i = 0; i < d_high; i++)
		{
			// index[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
			temp = counter_high[index_high[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		// if(mp[string(str)] == 1)
		return backup + min_value * (MAX_CNT_LOW + 1);
	}
	~BloomSketch();
};
#endif//_BloomSketch_SPLIT_H




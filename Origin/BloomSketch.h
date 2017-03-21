#ifndef _BloomSketch_H
#define _BloomSketch_H

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
	
	int *counter_low;
	int *carry_bit;
	int *counter_high;

	int w_low, d_low;
	int w_high, d_high;

	int MAX_CNT_LOW;
	int MAX_CNT_HIGH;
	// map<string, int> mp;

public:
	BloomSketch(int _w_low, int _w_high, int _d_low, int _d_high)
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
			index_high[i] = index_low[i] % w_high;
			index_low[i] %= w_low;

			temp = counter_low[index_low[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		int backup = min_value;


		if(min_value == MAX_CNT_LOW)
		{
			// cnt++;
			for(int i = 0; i < d_low; i++)
			{
				carry_bit[index_low[i]] = 1;
				counter_low[index_low[i]] = 0;
			}
			// mp[string(str)] = 1;
			
			//carry into the high counter!
			min_value = 1 << 30;
			for(int i = 0; i < d_high; i++)
			{
				// index[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;
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
		int min_value2 = 1 << 30;
		int temp;
		int flag = 1;

		for(int i = 0; i < d_low; i++)
		{
			// index[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			index_low[i] = (bobhash[i]->run(str, strlen(str)));
			index_high[i] = index_low[i] % w_high;
			index_low[i] %= w_low;
			
			temp = counter_low[index_low[i]];

			if(carry_bit[index_low[i]] == 0)
			{
				flag = 0;
				min_value = temp < min_value ? temp : min_value;
			}
			min_value2 = temp < min_value2 ? temp : min_value2;
		}
		if(flag == 0)
			return min_value2;

		int backup = min_value2;

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
#endif//_BloomSketch_H




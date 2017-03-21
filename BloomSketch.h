#ifndef _BLOOMSKETCH_H
#define _BLOOMSKETCH_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "BOBHash32.h"
#include "BOBHash64.h"
#include <iostream>
#include <string>
#include "params.h"
#include <map>


using namespace std;

typedef unsigned char uint8;
typedef unsigned short int uint16;

class BloomSketch
{	
private:
	BOBHash32 * bobhash[MAX_HASH_NUM];
	BOBHash64 * bobhash_bf;
	
	uint64 *counter_low;
	uint16 *counter_high;
	uint64 *bf;


	int w_low, d_low;
	int w_high, d_high;
	int w_bf, d_bf;

	int MAX_CNT_LOW;
	int MAX_CNT_HIGH;

public:
	int man_insert, man_query;
	BloomSketch(int _w_low, int _w_high, int _d_low, int _d_high, int _w_bf, int _d_bf)
	{
		man_insert = 0;
		man_query = 0;
		w_low = _w_low;
		w_high = _w_high;
		d_low = _d_low;
		d_high = _d_high;
		w_bf = _w_bf;
		d_bf = _d_bf;

		counter_low = new uint64[w_low >> 4];
		counter_high = new uint16[w_high];
		bf = new uint64[w_bf >> 6];

		memset(counter_low, 0, sizeof(uint64) * (w_low >> 4));
		memset(counter_high, 0, sizeof(uint16) * w_high);
		memset(bf, 0, sizeof(uint64) * (w_bf >> 6));

		MAX_CNT_LOW = (1 << LOW_COUNTER_SIZE) - 1;
		MAX_CNT_HIGH = (1 << HIGH_COUNTER_SIZE) - 1;

		for(int i = 0; i < d_low + d_high + d_bf; i++)
		{
			bobhash[i] = new BOBHash32(i + 1000);
		}
		bobhash_bf = new BOBHash64(d_low + d_high + 1000);
	}
	void Insert(const char * str)
	{
		int min_value = 1 << 30;
		
		uint temp;
		uint index_low[MAX_HASH_NUM];
		uint word_index[MAX_HASH_NUM];
		uint offset[MAX_HASH_NUM];
		uint index_high[MAX_HASH_NUM];
		uint index_bf[MAX_HASH_NUM];

		for(int i = 0; i < d_low; i++)
		{
			man_insert ++;

			index_low[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			word_index[i] = (index_low[i] >> 4);
			offset[i] = (index_low[i] & 0xF);

			temp = (counter_low[word_index[i]] >> (offset[i] << 2)) & 0xF;

			min_value = temp < min_value ? temp : min_value;
		}

		if(min_value != MAX_CNT_LOW)
		{
			for(int i = 0; i < d_low; i++)
			{
				temp = (counter_low[word_index[i]] >> (offset[i] << 2)) & 0xF;
				if(temp == min_value)
					counter_low[word_index[i]] += ((uint64)1 << (offset[i] << 2));
			}
			return;
		}




		for(int i = 0; i < d_low; i++)
		{
			counter_low[word_index[i]] &= (~((uint64)0xF << (offset[i] << 2)));
		}




		uint64 hash_value = bobhash_bf->run(str, strlen(str));
		word_index[0] = (hash_value & 0xFFFF) % (w_bf >> 6);
		hash_value >>= 16;

		for(int i = 0; i < d_bf; i++)
		{
			// index_bf[i] = (bobhash[i + d_low + d_high]->run(str, strlen(str))) % w_bf;
			// word_index[i] = (index_bf[i] >> 6);
			// offset[i] = (index_bf[i] & 0x3F);		
			offset[i] = (hash_value & 0x3F);
			hash_value >>= 6;

			bf[word_index[0]] |= ((uint64)1 << offset[i]);
		}
		man_insert ++;



		//carry into the high counter!
		min_value = 1 << 30;
		for(int i = 0; i < d_high; i++)
		{
			man_insert ++;
			index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;

			temp = counter_high[index_high[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		
		for(int i = 0; i < d_high; i++)
		{
			if(counter_high[index_high[i]] == min_value)
				counter_high[index_high[i]]++;
		}

		return;
	}
	int Query(const char *str)
	{
		uint min_value = 1 << 30;
		uint temp;		
		uint index_low[MAX_HASH_NUM];
		uint word_index[MAX_HASH_NUM];
		uint offset[MAX_HASH_NUM];
		uint index_high[MAX_HASH_NUM];
		uint index_bf[MAX_HASH_NUM];

		for(int i = 0; i < d_low; i++)
		{
			man_query ++;
			index_low[i] = (bobhash[i]->run(str, strlen(str))) % w_low;
			word_index[i] = (index_low[i] >> 4);
			offset[i] = (index_low[i] & 0xF);

			temp = (counter_low[word_index[i]] >> (offset[i] << 2)) & 0xF;

			min_value = temp < min_value ? temp : min_value;
		}



		uint64 hash_value = bobhash_bf->run(str, strlen(str));
		word_index[0] = (hash_value & 0xFFFF) % (w_bf >> 6);
		hash_value >>= 16;


		man_query ++;
		for(int i = 0; i < d_bf; i++)
		{
			// index_bf[i] = (bobhash[i + d_low + d_high]->run(str, strlen(str))) % w_bf;
			// word_index[i] = (index_bf[i] >> 6);
			// offset[i] = (index_bf[i] & 0x3F);
			offset[i] = (hash_value & 0x3F);
			hash_value >>= 6;

			if(((bf[word_index[0]] >> offset[i]) & 1) == 0)
				return min_value;
		}




		int backup = min_value;
		
		min_value = 1 << 30;
		for(int i = 0; i < d_high; i++)
		{
			man_query ++;
			index_high[i] = (bobhash[i + d_low]->run(str, strlen(str))) % w_high;

			temp = counter_high[index_high[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return backup + min_value * (MAX_CNT_LOW + 1);
	}
	~BloomSketch();
};
#endif//_BLOOMSKETCH_H




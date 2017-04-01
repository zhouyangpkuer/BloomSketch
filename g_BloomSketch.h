#ifndef _G_BLOOMSKETCH_H
#define _G_BLOOMSKETCH_H

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

class g_BloomSketch
{	
private:
	BOBHash32 * bobhash[MAX_HASH_NUM * 4];
	BOBHash64 * bobhash_bf[MAX_HASH_NUM];
	
	uint16 **counter;
	uint64 **bf;

	int w[MAX_HASH_NUM];
	int w_bf[MAX_HASH_NUM];
	int d[MAX_HASH_NUM];

	int size_counter[MAX_HASH_NUM];

	int num_layer;

	uint Hash_res[MAX_HASH_NUM];
	int max_d;
	uint64 Hash_resbf;

public:
	int man_insert, man_query;
	g_BloomSketch(int _num_layer, int* _w, int* _d, int* _w_bf, int* _size_counter)
	{
		man_insert = 0;
		man_query = 0;

		num_layer = _num_layer;
		counter = new uint16 *[num_layer];
		bf = new uint64 *[num_layer - 1];

		for(int i = 0; i < num_layer; i++)
		{
			w[i] = _w[i];
			d[i] = _d[i];
			size_counter[i] = _size_counter[i];

			counter[i] = new uint16[w[i]];
			memset(counter[i], 0, sizeof(uint16) * w[i]);

			if(i == num_layer - 1)
				break;

			w_bf[i] = _w_bf[i];
			bf[i] = new uint64[w_bf[i] >> 6];	
			memset(bf[i], 0, sizeof(uint64) * (w_bf[i] >> 6));
		}
		max_d = 0;
		for(int i = 0; i < num_layer; i++)
		{
			max_d = (max_d > d[i] ? max_d : d[i]);
		}

		for(int i = 0; i < max_d; i++)
		{
			bobhash[i] = new BOBHash32(i);
		}
		bobhash_bf[0] = new BOBHash64(500);

	}
	//true: overflow
	bool layer_insert(const char * str, int id)
	{
		int min_value = (1 << 30);
		int index[MAX_HASH_NUM];

		for(int i = 0; i < d[id]; i++)
		{
			man_insert ++;
			index[i] = Hash_res[i] % w[id];
			min_value = min_value < counter[id][index[i]] ? min_value : counter[id][index[i]];
		}
		if(min_value == ((1 << size_counter[id]) - 1))
		{
			man_insert ++;

			for(int i = 0; i < d[id]; i++)
			{
				counter[id][index[i]] = 0;
			}


			int word_index[MAX_HASH_NUM];
			int offset[MAX_HASH_NUM];

			if(id == 0)
				Hash_resbf = bobhash_bf[0]->run(str, strlen(str));

			uint64 hash_value = Hash_resbf;
			word_index[0] = (hash_value & 0xFFFF) % (w_bf[id] >> 6);
			hash_value >>= 16;

			for(int i = 0; i < d[id]; i++)
			{
				offset[i] = (hash_value & 0x3F);
				hash_value >>= 6;
				bf[id][word_index[0]] |= ((uint64)1 << offset[i]);
			}

			return true;
		}
		for(int i = 0; i < d[id]; i++)
		{
			if(min_value == counter[id][index[i]])
				counter[id][index[i]]++;
		}
		return false;
	}

	void Insert(const char * str)
	{
		for(int i = 0; i < max_d; i++)
		{
			Hash_res[i] = bobhash[i]->run(str, strlen(str));
		}

		int id = 0;
		while(layer_insert(str, id) == true)
		{
			id++;
		}
	}

	//true: overflow
	bool layer_query(const char *str, int id, int *result)
	{
		int min_value = (1 << 30);
		int index[MAX_HASH_NUM];

		for(int i = 0; i < d[id]; i++)
		{
			man_query ++;

			index[i] = Hash_res[i] % w[id];
			min_value = min_value < counter[id][index[i]] ? min_value : counter[id][index[i]];
		}
		*result = min_value;

		if(id == num_layer - 1)
			return false;


		int word_index[MAX_HASH_NUM];
		int offset[MAX_HASH_NUM];

		uint64 hash_value = Hash_resbf;
		word_index[0] = (hash_value & 0xFFFF) % (w_bf[id] >> 6);
		hash_value >>= 16;

		man_query ++;

		for(int i = 0; i < d[id]; i++)
		{			
			offset[i] = (hash_value & 0x3F);
			hash_value >>= 6;
			if(((bf[id][word_index[0]] >> offset[i]) & 1) == 0)
				return false;
		}
		return true;
	}


	int Query(const char *str)
	{
		for(int i = 0; i < max_d; i++)
		{
			Hash_res[i] = bobhash[i]->run(str, strlen(str));
		}
		Hash_resbf = bobhash_bf[0]->run(str, strlen(str));

		int final_result = 0, temp = 0;
		int id = 0;
		int cumu_size = 0;

		bool flag = layer_query(str, id, &temp);
		final_result += (temp << cumu_size);

		while(flag == true)
		{
			cumu_size += size_counter[id];
			id++;
			flag = layer_query(str, id, &temp);

			final_result += (temp << cumu_size);
		}
		return final_result;
	}
	~g_BloomSketch();
};

#endif//_G_BLOOMSKETCH_H

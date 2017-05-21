#ifndef _PARAMS_H
#define _PARAMS_H

// #define N_QUERY 620742
// #define N_INSERT 299887139
// const long long int N_INSERT = 299887139;

#define MAX_INSERT_PACKAGE 10000000
#define MAX_HASH_NUM 20
#define MAX_CARRY_LAYER 28
#define HASH_COUNTER 4

#define FILTER_SIZE 32

// #define TIMES 4
#define COEF 0.618

#define WORD_SIZE 64
#define COUNTER_SIZE 16

#define LOW_COUNTER_SIZE 4
#define HIGH_COUNTER_SIZE 12
#define LOW_HASH_NUM 4
#define HIGH_HASH_NUM 4


#define MAX_RE 4.0
#define DELTA MAX_RE / 30

typedef long long lint;
typedef unsigned int uint;
#endif //_PARAMS_H
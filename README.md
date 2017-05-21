# The Bloom sketch


## Introduction

*Sketch* is a probabilistic data structure, and is widely used for frequency estimation in data streams. One of the most significant metrics for such application is the accuracy, as this metric can reflect how well one sketch can summarize and depict the characteristics of data streams. The most common sketches used in data streams are the CM sketch and its several variants. However, given a limited memory size, these sketches always significantly overestimate some items and exhibit poor accuracy. To address this issue, we proposed a novel sketch named the Bloom sketch, combining the sketch with the Bloom filter, another well-known probabilistic data structure used for membership queries. Our proposed Bloom sketch achieves much higher accuracy compared with the state-of-the-art, while exhibiting comparable insertion and query speed. Extensive experiments based on real IP trace streams show that the accuracy is improved up to 14.47 times, compared with the CM sketch.

## About the source codes, dataset and parameters setting

We implement the CM, CU, C (short for the Count sketch) and Bloom sketch with C++. We complete these codes on Ubuntu 14.04 and compiled successfully using g++ 4.8.4.

The file stream.dat is the subset of one of the encrypted IP traces used in experiments. This small dataset contains 1M items totally and 193,894 distinct items. The maximum frequency of those items is 4426. The full dataset can be download on our homepage (http://net.pku.edu.cn/~yangtong/uploads/stream_full.dat).

We set the memory allocated to each sketch 0.1MB. The other parameters setting is the same as mentioned in the paper.


# How to run

Suppose you've already cloned the repository and start from the 'Bloom_Sketch_Framework' directory.

You just need:

	& make
	& ./main


## Output format

Our program will print the correct rate, the throughput of insertion and query of these sketches, the ARE and AAE of these sketches, and the average number of memory accesses of insertion and query of these sketch. Note that to obtain convincing results of the throughput, you are supposed to set the micro "testcycles" in the main.cpp to a larger value (e.g. 100) and to run the program on a Linux server.
CPPFLAGS = -Wall -O3 -std=c++11 -lm -w
PROGRAMS = main 

all: $(PROGRAMS)

main:main.cpp params.h BOBHash32.h ASketch.h CMSketch.h CUSketch.h CSketch.h BloomSketch.h \
	g_BloomSketch.h g_BloomSketch_omabf.h g_BloomSketch_reuse.h g_BloomSketch_omabf_reuse.h
	g++ -o main main.cpp $(CPPFLAGS)

clean:
	rm -f *.o $(PROGRAMS)

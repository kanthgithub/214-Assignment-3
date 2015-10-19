#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define malloc(x)	myMalloc(x, __FILE__, __LINE__)
#define free(x)		myFree((void*)x, __FILE__, __LINE__)

# define blocksize 1<<20
# define fudge_factor 5
# define recpattern 0xAAAAAAAA

typedef struct mementry
{
	struct mementry *prev, *succ;
	int recognize;
	int isfree;
	unsigned int size;
} mementry;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define malloc(x)	myMalloc(x, __FILE__, __LINE__)
#define free(x)		myFree((void*)x, __FILE__, __LINE__)

# define blocksize 1<<20
# define fudge_factor 5
# define recpattern 0xAAAAAAAA
//Mementry structure- INCLUDE IN HEADER FILE

typedef struct mementry
{
	struct mementry *prev, *succ;
	int recognize;
	int isfree;
	unsigned int size;
} mementry;

static char big_block[blocksize];

//static int initialized=0;

void *myMalloc(unsigned int size, char * file, int line)
{
	if(size>((blocksize)-24))
	{
		printf("TOO LARGE CHUNK REQUESTED\n");
		printf("Call specifics: FILE %s LINE %d\n", file, line);
		return(0);
	}

	static int initialized=0;
	static mementry *root;
	mementry *p, *succ, *prev;

	if(!initialized)
	{
		root=(mementry*)big_block;
		root->prev=NULL;
		root->succ=NULL;
		root->recognize=recpattern;
		root->size=(blocksize)-sizeof(mementry);
		root->isfree=1;
		initialized=1;
		printf("Dynamic memory slot initialized with root at %d \n", root);
	}
	
	p=root;
	prev=root;
	//mementry*test=root;
	//printf("%d is the address of root \n",root);
	while(p!=NULL)
	{
		if(p->size < size)
		{
			prev=p;
			
			p=p->succ;
			
		}
		else if(!p->isfree)
		{
			prev=p;
			
			p=p->succ;
		
		}
		else if(p->size <= (size + sizeof(mementry)))
		{
			p->isfree=0;
			p->prev=prev;
			
			p->recognize=recpattern;
			printf("Memory allocated to existing mementry. Location of mementry is %d\n", p);
			printf("Size of mementry is actually %d while size requested was %d \n",p->size, size );
			printf("p has a prev as %d and successor as %d \n",p->prev, p->succ );
			printf("p isfree is %d and p recognize is %x \n",p->isfree, p->recognize );
			printf("\n\n");
			return (char*)p + sizeof(mementry);
			//return p + sizeof(mementry);
		}
		else
		{
			printf("%d\n", __LINE__);
			succ=(mementry *)((char *)p + sizeof(mementry) + size);
			succ->prev=p;
			succ->succ=p->succ;
			if(p->succ!=NULL)
			{
				p->succ->prev=succ; //This is just p = succ 
				printf("%d\n", __LINE__);
			}
			p->succ=succ;
			succ->size= p->size - sizeof(mementry) - size;
			succ->isfree=1;
			p->size=size;
			p->isfree=0;
				
			p->prev=prev;		

			p->recognize=recpattern;	
			//printf("%d p itself\n", p+sizeof(mementry));
			//printf("%d prev\n", p->prev);
			//printf("%d root\n", root);
			//printf("%d root isfree\n", root->isfree);
			//printf("%d\n __LINE__", p->prev->isfree); //thisbreaksus

			printf("Memory allocated, new mementry created. Location of mementry is %d\n", p);
			printf("Size of mementry is actually %d while size requested was %d \n",p->size, size );
			printf("p has a prev as %d and successor as %d \n",p->prev, p->succ );
			printf("p isfree is %d and p recognize is %x \n",p->isfree, p->recognize );
			printf("\n\n");
			
			return (char*)p + sizeof(mementry);
			//return p + sizeof(mementry);
		}
	}

	printf("ERROR: USED ALL MEMORY");	
	return 0;
}
void myFree(void * p1, char * file, int line)
{
	mementry *ptr, *pred, *succ;
	ptr = (mementry *)p1-1;
	//ptr = (mementry *)((char*)p1-sizeof(mementry)); 
	
	printf("ptr %d isfree is %d \n", ptr, ptr->isfree);	
	printf("ptr %d size %d \n", ptr, ptr->size);	
	printf("ptr %d recognize %x \n", ptr, ptr->recognize);	
	
	
	if(ptr->recognize!=recpattern)
	{
		printf("\x1B[2;31m ERROR: %s Line %d NOT ALLOCATED FROM MALLOC\n \x1B[0m", file, line);
		return;
	}

	if(ptr->isfree)
	{
		printf("\x1B[2;31m ERROR: %s Line %d ALREADY DEALLOCATED\n \x1B[0m",file,line);
		return;
	}


	if(((pred=ptr->prev)!=NULL) && pred->isfree)
	{
				
		pred->size += sizeof(mementry) + ptr->size;
		pred->succ = ptr->succ;
		
		if(ptr->succ!=NULL)
			ptr->succ->prev = pred;
		
	}
	else
	{
		ptr->isfree = 1;
		pred = ptr;
	}
	if((succ=ptr->succ)!=NULL && succ->isfree)
	{
		
		pred->size += sizeof(mementry) + succ->size;
		pred->succ = succ->succ;
		
		if(succ->succ!=NULL)
			succ->succ = pred;
	}
	printf("Free was successful.\n");
}

int main()
{
	printf("Size of mementry is %d \n", sizeof(mementry));
	int *arr= (int *)malloc(10*sizeof(int));
	printf("Malloc for arr gave me the address %d \n",arr);
	int i = 0;
	for(i=0; i<10; i++)
		arr[i]=i+1;
	printf("Allocated an array of 10 integers to arr.\n");
	// i=0;
	// while(i<10){
	// printf("%d\n", arr[i]);
	// i++;
	// }
	
	char *b= (char *)malloc(10*sizeof(char));
	printf("Malloc for b gave me the address %d\n", b);
	int g = 0;
	for(g=0; g<9; g++)
		b[g]='r';
	b[9]='\0';
	printf("b stores %s in address %d\n",b, b);
	
	//Freeing b+10, which wasnt an address by our malloc SHOULD GIVE AN ERROR
	free(b+10);
	
	//Freeing b normally, should be successful.
	free(b);

	//Freeing b again; SHOULD GIVE AN ERROR
	free(b);

	//TRYING TO FREE SOMETHING NOT ALLOCATED BY MALLOC
	int x;
	free(&x);
		
	return(0);
}

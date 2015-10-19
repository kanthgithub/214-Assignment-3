#include "header.h"

static char big_block[blocksize];
static int memused=0;

void *myMalloc(unsigned int size, char * file, int line)
{
	if(size>((blocksize)-24))
	{
		printf("TOO LARGE CHUNK REQUESTED\n");
		printf("Call specifics: FILE %s LINE %d\n", file, line);
		return(0);
	}

	//static int memused=0;
	static int initialized=0;
	static mementry *root;
	
	//THIS IS AN IMPORANT VARIABLE.
	//When we get our first request for a memory chunk of 1000 bytes or larger, this will be initialized to hold that first LARGE CHUNK at
	//the opposite end of the array. It's kept uninitialized UNTIL OUR FIRST LARGE CHUNK REQUEST.
	static mementry *last;


	mementry *p, *succ, *prev;

	printf("MEMUSED SO FAR: %d MEMLEFT: %d\n", memused, ((blocksize)-24)-memused);

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

	//THIS IS AN IMPORTANT VARIABLE.
	//lr is our "LARGE CHUNK OR NOT" variable. It's set to 0 when we deal with small chunks, and 1 when we deal with large chunks. 
	//When it's set to 1, several if statements in the while loop starting at line 112 will change the behavior of the loop so as to 
	//properly handle the situation.
	int lr=0;

	if(size>=1000)
	{
		//I leave last completely NULL until we get our first Large Chunk Request. Again, last acts as our "handle" onto the opposite end of the
		//array. Using it, we keep our Large Chunk Space growing from right to left, meeting the smaller chunks in the middle of the array.
		if(last==NULL)
		{

			printf("%d in last==NULL\n", __LINE__);
			last=((mementry *)(&big_block[blocksize-1])-(sizeof(mementry)+size)); //placing last at the end of the array, with our first large chunk

			//I need to traverse the old list, starting from root, so that I can properly "hook up" LAST.			
			p=root;
			while(p->succ!=NULL)
				p=p->succ;
			
			p->succ=last;
			last->prev=p;
			last->succ=NULL;
			last->isfree=0;
			last->size=size;
			last->recognize=recpattern;

			printf("last is at: %d\n last-prev is: %d\n last-size is: %d\n p-size %d\n p-succ %d\n p %d\n", last, last->prev, last->size, p->size, p->succ, 				p);
			return last;
		}
		else
		{
			//We come in here if we've already had at least one Large Chunk request. 
			//I set p and prev equal to last so that I can traverse the list
			//BACKWARDS.
			p=last;
			prev=last;
			lr=1;
			//ALSO: notice I set lr=1. This is what tells our while loop below whether it's dealing with a large chunk or not
			//And allows us to deal with it approriately.
		}
	}	
	
	//Again, lr is the variable that tells the program whether we're dealing with a large chunk or not. If it isn't, lr==0, and we proceed
	//normally: going from root to the end of the Linked list.		
	if(lr==0)
	{
		p=root;
		prev=root;
	}
	//mementry*test=root;
	//printf("%d is the address of root \n",root);
	while(p!=NULL)
	{
		if(p->size < size)
		{
			
			prev=p;
			if(lr==1)
				p=p->prev;
			else	
				p=p->succ;
			
		}
		else if(!p->isfree)
		{
			prev=p;
			if(lr==1)
				p=p->prev;
			else
				p=p->succ;
		
		}
		else if(p->size <= (size + sizeof(mementry)))
		{
			p->isfree=0;
			if(lr!=1)
				p->prev=prev;
			else
				p->succ=prev;
			
			p->recognize=recpattern;
			printf("Memory allocated to existing mementry. Location of mementry is %d\n", p);
			printf("Size of mementry is actually %d while size requested was %d \n",p->size, size );
			printf("p has a prev as %d and successor as %d \n",p->prev, p->succ );
			printf("p isfree is %d and p recognize is %x \n",p->isfree, p->recognize );
			printf("\n\n");
			memused=memused+size;
			return (char*)p + sizeof(mementry);
			//return p + sizeof(mementry);
		}
		else
		{
			//Saturation 
			if(memused+size>((blocksize)-24))
			{
				printf("\x1B[2;31m ERROR: SATURATION, NO ROOM FOR REQUESTED CHUNK. FILE %s LINE %d\n \x1B[0m", file, line);
				return NULL;
			}
			memused=memused+size;
			
			printf("%d", __LINE__);	

			//Here, we see lr again. It tells the program whether we're dealing with a large chunk or not, if lr==0 we proceed normally, 
			//if lr==1 then we're going in reverse (starting at the "LAST" node at the opposite end of the array and proceeding backwards).
			if(lr==0)
			{
				printf("%d\n", __LINE__);

				succ=(mementry *)((char *)p + sizeof(mementry) + size);
				succ->prev=p;
				succ->succ=p->succ;

				if(p->succ!=NULL)
				{
					p->succ->prev=succ;  
					printf("%d\n", __LINE__);
				}

				p->succ=succ;
				succ->size= p->size - sizeof(mementry) - size;
				succ->isfree=1;
				p->size=size;
				p->isfree=0;
				p->prev=prev;		
				p->recognize=recpattern;	
			}
			else if(lr==1)
			{
				//Since we are going from the other end of the array, SUCC becomes the smaller portion of the splitting, and it holds our input data.
				//As you might notice, I just reversed a lot of what we have above.
				printf("%d\n", __LINE__);
				succ=((mementry *)((char *)p + p->size) - size);
				succ->prev=p;
				succ->succ=p->succ;
				if(p->succ!=NULL)
					p->succ->prev=succ; 

				p->succ=succ;
				succ->size=size;
				succ->isfree=0;
				p->size=p->size - sizeof(mementry) - size;
				p->isfree=1;
				p->prev=prev;
				succ->recognize = recpattern;

				//You'll notice that the actual address of succ is a little weird here.	
				printf("succ %d, succ->size %d, succ-prev %d, succ-succ %d\n, p %d, p-succ %d\n", succ, succ->size, succ->prev, succ->succ, p, p->succ);
			}

			//printf("%d p itself\n", p+sizeof(mementry));
			//printf("%d prev\n", p->prev);
			//printf("%d root\n", root);
			//printf("%d root isfree\n", root->isfree);
			//printf("%d\n __LINE__", p->prev->isfree);

			printf("Memory allocated, new mementry created. Location of mementry is %d\n", p);
			printf("Size of mementry is actually %d while size requested was %d \n",p->size, size );
			printf("p has a prev as %d and successor as %d \n",p->prev, p->succ );
			printf("p isfree is %d and p recognize is %x \n",p->isfree, p->recognize );
			printf("\n\n");
			
			if(lr==1)
				return (char *)succ + sizeof(mementry);

			return (char*)p + sizeof(mementry);
			//return p + sizeof(mementry);
		}
	}

	printf("\x1B[2;31m ERROR: SATURATION, NO ROOM FOR REQUESTED CHUNK. CALL SPECIFICS: FILE: %s, LINE: %d\n \x1B[0m", file, line);	
	return 0;
}
void myFree(void * p1, char * file, int line)
{
	//NOTE: I'M NOT SURE HOW MY FRAGMENTATION APPROACH AFFECTS FREE. I'm assuming it should be fine, though. From free's perspective
	//everything should be the same, we're stil just passing it one linked list.

	mementry *ptr, *pred, *succ;
	ptr = (mementry *)p1-1;
	//ptr = (mementry *)((char*)p1-sizeof(mementry)); 
	
	printf("IN FREE\n");
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

	memused=memused-(ptr->size);	
	printf("MEMLEFT AFTER FREEING: %d\n", (((blocksize)-24)-memused));

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
	

	char *b= (char *)malloc(10*sizeof(char));
	printf("Malloc for b gave me the address %d\n", b);
	int g = 0;
	for(g=0; g<9; g++)
		b[g]='r';
	b[9]='\0';
	printf("b stores %s in address %d\n",b, b);

	printf("in main, after all mallocs:MEMUSED: %d MEMLEFT: %d\n", memused, ((blocksize)-24)-memused);

	//adding two "large" chunks. 	
	char *c= (char *)malloc(1000);
	char *d= (char *)malloc(1000);
	
	//Freeing b+10, which wasnt an address by our malloc SHOULD GIVE AN ERROR
	free(b+10);
	
	//Freeing b normally, should be successful.
	free(b);

	//Freeing b again; SHOULD GIVE AN ERROR
	free(b);

	//Trying to free one of our LARGE CHUNKS
	free(d);

	//TRYING TO FREE SOMETHING NOT ALLOCATED BY MALLOC
	int x;
	free(&x);
	
	return(0);
}




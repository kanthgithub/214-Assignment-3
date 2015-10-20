#include "header.h"

static char big_block[blocksize];
static int memused=0;

void *myMalloc(unsigned int size, char * file, int line)
{
	if(size>((blocksize)-sizeof(mementry)))
	{
		printf("TOO LARGE CHUNK REQUESTED\n");
		printf("Call specifics: FILE %s LINE %d\n", file, line);
		return(0);
	}

	static int initialized=0;
	static mementry *root;
	
	//THIS IS AN IMPORANT VARIABLE.
	//When we get our first request for a memory chunk of 1000 bytes or larger, this will be initialized to hold that first LARGE CHUNK at
	//the opposite end of the array. It's kept uninitialized UNTIL OUR FIRST LARGE CHUNK REQUEST.
	static mementry *last;


	mementry *p, *succ, *prev;

	printf("MEMUSED SO FAR: %d MEMLEFT: %d\n", memused, ((blocksize)-sizeof(mementry))-memused);

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
		}
		else
		{
			//Saturation 
			if(memused+size>((blocksize)-sizeof(mementry)))
			{
				printf("\x1B[2;31m ERROR: SATURATION, NO ROOM FOR REQUESTED CHUNK. FILE %s LINE %d\n \x1B[0m", file, line);
				return NULL;
			}
			memused=memused+size;
			//Here, we see lr again. It tells the program whether we're dealing with a large chunk or not, if lr==0 we proceed normally, 
			//if lr==1 then we're going in reverse (starting at the "LAST" node at the opposite end of the array and proceeding backwards).
			if(lr==0)
			{
				succ=(mementry *)((char *)p + sizeof(mementry) + size);
				succ->prev=p;
				succ->succ=p->succ;

				if(p->succ!=NULL)
				{
					p->succ->prev=succ; 
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
			}
			printf("Memory allocated, new mementry created. Location of mementry is %d\n", p);
			printf("Size of mementry is actually %d while size requested was %d \n",p->size, size );
			printf("p has a prev as %d and successor as %d \n",p->prev, p->succ );
			printf("p isfree is %d and p recognize is %x \n",p->isfree, p->recognize );
			printf("\n\n");
			
			if(lr==1)
				return (char *)succ + sizeof(mementry);

			return (char*)p + sizeof(mementry);
		}
	}

	printf("\x1B[2;31m ERROR: SATURATION, NO ROOM FOR REQUESTED CHUNK. CALL SPECIFICS: FILE: %s, LINE: %d\n \x1B[0m", file, line);	
	return 0;
}
void myFree(void * p1, char * file, int line)
{
	mementry *ptr, *pred, *succ;
	ptr = (mementry *)p1-1;	

	// COMMENTED OUT PRINTF FOR DEBUGGING
	// printf("IN FREE\n");
	// printf("ptr %d isfree is %d \n", ptr, ptr->isfree);	
	// printf("ptr %d size %d \n", ptr, ptr->size);	
	// printf("ptr %d recognize %x \n", ptr, ptr->recognize);	
	
	
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
	printf("MEMLEFT AFTER FREEING: %d\n", (((blocksize)-sizeof(mementry))-memused));

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
	int *arr= (int *)malloc(10*sizeof(int));
	int i = 0;
	for(i=0; i<10; i++)
		arr[i]=i+1;
	printf("Allocated an array of 10 integers to arr.\n");
	char *b= (char *)malloc(10*sizeof(char));
	int g = 0;
	for(g=0; g<9; g++)
		b[g]='r';
	b[9]='\0';
	printf("b stores %s in address %d\n",b, b);

	printf("in main, after all mallocs:MEMUSED: %d MEMLEFT: %d\n", memused, ((blocksize)-sizeof(mementry))-memused);

	printf("Adding two LARGE chunks now\n");
	char *c= (char *)malloc(1000);
	char *d= (char *)malloc(1000);
	
	printf("Freeing b+10, which wasnt an address by our malloc SHOULD GIVE AN ERROR\n");
	free(b+10);
	
	printf("Freeing b normally, should be successful.\n");
	free(b);

	printf("Freeing b again; SHOULD GIVE AN ERROR\n");
	free(b);

	printf("Trying to free one of our LARGE CHUNKS\n");
	free(d);

	printf("TRYING TO FREE SOMETHING NOT ALLOCATED BY MALLOC\n");
	int x;
	free(&x);
	
	return(0);
}




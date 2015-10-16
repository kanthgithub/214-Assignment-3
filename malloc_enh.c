#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define malloc(x)	myMalloc( x, __FILE__, __LINE__)
#define free(x)		myFree(x, __FILE__, __LINE__)

# define blocksize 1<<20
# define fudge_factor 5

//Mementry structure- INCLUDE IN HEADER FILE

typedef struct mementry
{
	struct mementry *prev, *succ;
	int isfree;
	unsigned int size;
	//int recognize;
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
		root->size=(blocksize)-sizeof(mementry);
		root->isfree=1;
		initialized=1;
	}
	
	p=root;
	prev=root;
	mementry*test=root;
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
			//return (char*)p + sizeof(mementry);
			return p + sizeof(mementry);
		}
		else
		{
			printf("%d\n", __LINE__);
			succ=(mementry *)((char *)p + sizeof(mementry) + size); // NOT CORRECT??? IN RECITATION PEOPLE SUGGESTED TO PUT size IN PARENTHESIS!
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
			printf("%d p itself\n", p+sizeof(mementry));
			printf("%d prev\n", p->prev);
			printf("%d root\n", root);
			printf("%d root isfree\n", root->isfree);
			//printf("%d\n __LINE__", p->prev->isfree); //thisbreaksus

			//return (char*)(p + sizeof(mementry));
			return p + sizeof(mementry);
		}
	}

	printf("ERROR: USED ALL MEMORY");	

	return 0;
}
void myFree(void * p1, char * file, int line)
{
	mementry *ptr, *pred, *succ;
	printf("%d p1\n", p1);

	ptr = (mementry *)p1-1; 
	
	printf("%d is adress of ptr \n", ptr);
	printf("%d\n",__LINE__);

	printf("%d is ptr->prev\n", ptr->prev);
	printf("%d\n",__LINE__);	//stops here

	if(((pred=ptr->prev)!=NULL) && pred->isfree)
	{
		printf("%d\n",__LINE__);		
		pred->size += sizeof(mementry) + ptr->size;
		pred->succ = ptr->succ;
		printf("%d\n", __LINE__);
		if(ptr->succ!=NULL)
		{
			printf("%d \n", __LINE__); //
			ptr->succ->prev = pred;
		}
		printf("%d\n", __LINE__);
	}
	else
	{
		printf("%d\n",__LINE__);
		ptr->isfree = 1;
		pred = ptr;
	}
	if((succ=ptr->succ)!=NULL && succ->isfree)
	{
		printf("%d\n",__LINE__);
		pred->size += sizeof(mementry) + succ->size;
		pred->succ = succ->succ;
		
		if(succ->succ!=NULL)
			succ->succ = pred;
	}

	// I DONT KNOW IF THIS IS COMPLETE? ThIS IS THE LAST THING HE WROTE IN CLASS
}

int main()
{
	int *arr= (int *)malloc(10*sizeof(int));
	
	printf("%d adress of arr\n", arr);

	arr[0]=1;
	int i = 0;
	for(i=0; i<10; i++)
		arr[i]=i+1;
	i=0;
	while(i<10){
	printf("%d\n", arr[i]);
	i++;
	}
	
	char *b= (char *)malloc(10*sizeof(char));
	printf("%d b before\n", b);
	//printf("%d", __LINE__);
	*b='a';
	//int g = 0;
	//for(g=0; g<10; g++)
	//	b[g]='a'
	printf("%d \n", __LINE__);
	printf("%c %d\n",*b, b);
	
	free(b);
					//I managed to fix it by not adding any values to B. By adding values to B it for some reason changed the address completely.
	//printf("%s\n", b);	
	return(0);
}


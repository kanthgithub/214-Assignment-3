#include <stdio.h>
#include <stdlib.h>

#define malloc( x )	myMalloc( x, __FILE__, __LINE__)
#define free( x )	myFree(x, __FILE__, __LINE__)

# define blocksize 1<<20
# define fudge_factor 5

//Mementry structure- INCLUDE IN HEADER FILE

typedef struct mementry
{
	struct mementry *prev, *succ;
	int isfree;
	unsigned int size;
} mementry;

static char big_block[blocksize];

//static int initialized=0;

void *mymalloc(unsigned int size)
{
	if(size>((blocksize)-24))
	{
		printf("TOO LARGE CHUNK REQUESTED\n");
		return(0);
	}
	static int initialized=0;
	static mementry *root;
	mementry *p, *succ;

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
	mementry*test=root;
	printf("%d is the address of root \n",root);
	do
	{
		if(p->size < size)
			p=p->succ;
		else if(!p->isfree)
			p=p->succ;
		else if(p->size < (size + sizeof(mementry) + fudge_factor))
		{
			p->isfree=0;
			return (char*)p + sizeof(mementry);
		}
		else
		{
			succ=(mementry*)((char*)p + sizeof(mementry)) + size; // NOT CORRECT??? IN RECITATION PEOPLE SUGGESTED TO PUT size IN PARENTHESIS!
			succ->prev=p;
			succ->succ=p->succ;
			if(p->succ!=NULL)
				p->succ->prev=succ;
			p->succ=succ;
			succ->size=p->size - sizeof(mementry) - size;
			succ->isfree=1;
			p->size=size;
			p->isfree=0;
			return (char*)p + sizeof(mementry);
		}
	}while(p!=NULL);

	printf("ERROR: USED ALL MEMORY");	

	return 0;
}
void myfree(void * p1)
{
	mementry *ptr, *pred, *succ;

	ptr = (mementry *)((char*)p1-sizeof(mementry));

	printf("%d is adress of ptr->prev \n", ptr->prev);
	printf("%d\n",__LINE__);

	if((pred=ptr->prev)!=NULL&& pred->isfree)
	{
		printf("%d\n",__LINE__);		
		pred->size += sizeof(mementry) + ptr->size;
		pred->succ = ptr->succ;
		if(ptr->succ!=NULL)
			ptr->succ->prev = pred;
	}
	else
	{
		printf("%d\n",__LINE__);
		ptr->isfree =1;
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
	int* arr=(int*)mymalloc(10*sizeof(int));
	arr[0]=1;
	int i = 0;
	for(i=0; i<10; i++)
		arr[i]=i+1;
	i=0;
	while(i<10){
	printf("%d\n", arr[i]);
	i++;
	}
	
	char *b= (char *)mymalloc(10*sizeof(char));
	//printf("%d", __LINE__);
	b="abcdef";
	printf("%s\n", b);
	printf("%d size of char \n", sizeof(mementry));	
	mementry *x =(mementry *)(arr-6);
	printf("%d is the address of x \n", x);
	printf("%d is the free of x \n", x->isfree);
	myfree(b);
	printf("%d is the isfree of x after freeing it\n", x->isfree);
	printf("%d\n", __LINE__);
	//printf("%s\n", b);	
	return(0);
}

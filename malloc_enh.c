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

void *mymalloc(unsigned int size)
{
	static int initialized=0;
	mementry *root;
	mementry *p, *succ;

	if(!initialized)
	{
		root=(mementry*)big_block;
		root->prev=NULL;
		root->succ=NULL;
		root->size=blocksize-sizeof(mementry);
		root->isfree=1;
		initialized=1;
	}
	
	p=root;
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
	return 0;
}
void myfree(void * p1)
{
	mementry *ptr, *pred, *succ;
	ptr = (mementry*)p1 - 1;

	if((pred=ptr->prev)!=NULL&& pred->isfree)
	{
		pred->size += sizeof(mementry) + ptr->size;
		pred->succ = ptr->succ;
		if(ptr->!=NULL)
			ptr->succ->prev = pred;
	}
	else
	{
		ptr->isfree =1;
		pred = ptr;
	}
	if((succ=ptr->succ)!=NULL && succ->isfree)
	{
		pred->size += sizeof(mementry) + succ->size;
		pred->succ = succ->succ;
		if(succ->succ!=NULL)
			succ->succ = pred;
	}
	// I DONT KNOW IF THIS IS COMPLETE? ThIS IS THE LAST THING HE WROTE IN CLASS
}
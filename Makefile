CC=gcc
CFlags=-g -Wall -Wstrict -prototypes
PFlags=-g -C - H -Rw

all: implementation

implementation: implementation.c
	$(CC) -o implementation implementation.c header.h 

clean: 
	rm implementation

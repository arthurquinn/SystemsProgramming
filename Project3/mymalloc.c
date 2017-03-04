/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "mymalloc.h"

void * mymalloc(size_t size, char * file, unsigned int line)
{

	MemEntry * me = (MemEntry *) memspace; //
	MemEntry * temp = NULL;
	MemEntry newme = {CODE, 0, 0, 0, NULL, NULL}; //newme will be used to insert new MemEntry structs into memspace
	int remaining = 0; //will be used to store remaining free space referenced by MemEntry structs
	if(size < 1 || size > CAPACITY - sizeof(MemEntry))
	{
		//Error to be returned if an invalid size is passed to mymalloc
		printf("Error: File:%s Line:%u in function 'mymalloc':\n\tInvalid size given to mymalloc. ", file, line);
		printf("Size must be greater than 0 and less than max capacity. Returned NULL.\n");
		return NULL;
	}
	//if the special code was found this implies a MemEntry struct already exists at the beginning of memspace
	if(me->specialcode == CODE)
	{
		while(me != NULL)
		{
			temp = me;
			if(me->previous != NULL)
				if(me->arrayno != me->previous->arrayno) newme.arrayno = me->arrayno;
			//if me is free and there is enough room for the memory request, an new MemEntry and at least one free byte
			if(me->isfree && me->numbytes > size + sizeof(MemEntry) + 1)
			{
				//splits the current space referenced by MemEntry and stores a new MemEntry referencing the free space following
				temp++;
				temp = (MemEntry *)((char *) temp + size);
				remaining = me->numbytes - size;
				me->isfree = 0;
				me->numbytes = size;
				newme.numbytes = remaining - sizeof(MemEntry);
				newme.isfree = 1;
				newme.next = me->next;
				newme.previous = me;
				*temp = newme;
				if(me->next != NULL) me->next->previous = temp;
				me->next = temp;
				printf("Allocating %zu bytes at address %p\n", size, me + 1);
				return me + 1;
			}
			//if me is free and there is enough room for the memory request but NOT a new MemEntry struct, simply reuse this free MemEntry
			else if(me->isfree && me->numbytes >= size)
			{
				//simple case...just set isfree to 0 and re-use old MemEntry for this new request
				me->isfree = 0;
				printf("Allocating %zu bytes at address %p\n", size, me + 1);
				return me + 1;
			}
			else if(size > CAPACITY - sizeof(MemEntry))
			{
				//error to be printed if the size requested is greater than the maximum capacity of memspace
				printf("Error: File:%s Line:%u in function 'mymalloc':\n\tSize request cannot be greater than max CAPACITY. ", file, line);
				printf("CAPACITY is CAPACITY - sizeof(MemEntry) ... 5000 - 40 = 4960\n");
				return NULL;
			}
			me = me->next;
		}
		//Warning that will be printed to signal that a new block of 5000 bytes will be created using sbrk (this is an extra credit attempt)
		printf("Warning: File:%s Line:%u in function 'mymalloc':\n\tNo available space left to allocate. ", file, line);
		printf("Size requested was larger than available space.\n\tIncreasing data segment using SBRK.\n");
	}
	//if a special code was not found at the beginning of the memspace array, this implies that this was the first MemEntry stored within memspace
	//as this is the first entry in memspace we must create a new MemEntry at the beginning of memspace, and another MemEntry after the block of
	//memory requested by the user
	//this second MemEntry will be flagged as free and signal the remaining free space within memspace
	else
	{
		//first check that the requested size is valid
		if(size <= CAPACITY - sizeof(MemEntry))
		{
			//if  valid, store a new MemEntry struct at the beginning of memspace
			newme.numbytes = size;
			*me = newme;
			temp = me + 1; //set temp to point to space requested by user
			temp = (MemEntry *) ((char *) temp + size); //advance temp to end of space requested by user
			remaining = CAPACITY - (sizeof(MemEntry) + size); //set remaining space
			//this MemEntry struct will represent the remaining free space left in memspace
			if(remaining > sizeof(MemEntry) + 1) //if there is enough remaining space for another MemEntry and at least one free byte
			{
				newme.numbytes = remaining - sizeof(MemEntry);
				newme.isfree = 1;
				newme.previous = me;
				*temp = newme;
				me->next = temp; //set me next to temp
			}
			printf("Allocating %zu bytes at address %p\n", size, me + 1);
			return me + 1;
		}
		//error to be printed if the size requested is greater than the max capacity allowed
		printf("Error: File:%s Line:%u in function 'mymalloc':\n\tSize requested greater than max capacity allowed. ", file, line);
		printf("Increasing data segment using sbrk.\n");
	}
	//this block will handle calls that will not fit in the current data segment
	//sbrk will return another 5000 char array to store new MemEntry data structs and their corresponding data
	//this new 5000 char array will be included in the linked list with all previous MemEntry data structs
	//note that the max continuous size of each of these char arrays is 5000, so the maximum data entry of any call
	//can only be CAPACITY - sizeof(MemEntry) which is 4960 bytes.
	temp->next = sbrk(CAPACITY);	
	newme.numbytes = CAPACITY - sizeof(MemEntry);
	newme.isfree = 1;
	newme.arrayno++;
	newme.previous = temp;
	*(temp->next) = newme;
	return mymalloc(size, file, line);
}

void * mycalloc(size_t size, char * file, unsigned int line)
{
	//mycalloc is relatively simple, it simply uses the function mymalloc to get a pointer to free space of the size requested by the user
	//then, if mymalloc was successful, it sets this space to all 0's using the memset function
	void * ptr = mymalloc(size, file, line);
	if(ptr != NULL)
	{
		memset(ptr, 0, size);
		return ptr;
	}
	else
	{
		//error to be printed if there was an error in the mymalloc call
		printf("Error: File:%s Line:%u in function 'mycalloc':\n\tError in 'mymalloc' call\n", file, line);
	}
}

int myfree(void * ptr, char * file, unsigned int line)
{
	MemEntry * me = (MemEntry *) ptr;
	MemEntry * temp = NULL;
	int newsize = 0;
	unsigned int getcode; //this willl be used to check if a valid code exists at the front of a MemEntry
	me--;
	getcode = *((unsigned int *) me); //check if code at start of MemEntry struct is valid
	if(getcode == CODE)
	{
		if(me->isfree)
		{
			//if the code was valid but this MemEntry struct was already freed, we return an error stating that myfree cannot free the same pointer twice
			printf("Error: File:%s Line:%u in function 'myfree':\n\tPointer was already freed. ", file, line);
			printf("Function 'myfree' cannot free a deallocated pointer.\n");
			return 0;
		}
		if(me->previous != NULL)
		{
			if(me->previous->isfree && me->previous->arrayno == me->arrayno)
			{
				//if the previous node is not null and if it is free and in the same 5000  byte block of contiguous memory
				//then the current MemEntry struct will be merged with the previous
				temp = me->previous;
				newsize = me->numbytes + me->previous->numbytes + sizeof(MemEntry);
				me->previous->next = me->next;
				me->next->previous = me->previous;
				memset((void *) me, 0, sizeof(MemEntry)); //clears out freed MemEntry
				me = temp;
				me->numbytes = newsize;
			}
		}
		if(me->next != NULL)
		{
			if(me->next->isfree && me->next->arrayno == me->arrayno)
			{
				//similar to the above conditions (except in this case with the next MemEntry struct in the list) 
				//the current MemEntry struct will be merged with the next one
				temp = me->next;
				newsize = me->numbytes + me->next->numbytes + sizeof(MemEntry);
				me->next = me->next->next;
				memset((void *) temp, 0, sizeof(MemEntry)); //clears out freed MemEntry
				me->numbytes = newsize;
			}
		}
		me->isfree = 1; //set me isfree to 1
		//print message stating that memory was successfully freed at this address
		printf("Freed space at address %p\n", ptr);
		return 1;
	}
	else
	{
		//error to be printed if myfree received a pointer that did not point to a valid memory address
		//valid memory addresses are only memory addresses that were returned by mymalloc
		printf("Error: File:%s Line:%u in function 'myfree':\n\tCannot free invalid pointer. ", file, line);
		printf("Only pass pointers allocated by mymalloc.\n");
		return 0; //free is unsuccessful
	}
}

//the PrintList function is used to give a visual representation of the order and structure of the MemEntry list
void PrintList()
{
	MemEntry * me = (MemEntry *) memspace;
	int i = 0;
	while(me != NULL)
	{
		printf("|NumBytes: %zu\tIsFree: %d|-->", me->numbytes, me->isfree);
		me = me->next;
		if(me != NULL) printf("\n");
	}
	printf(" NULL\n");
	return;
}
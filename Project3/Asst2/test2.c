/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test2: Demonstrate data saturation:\n");
	void * ptr1 = malloc(2000);
	void * ptr2 = malloc(2000);
	void * ptr3 = malloc(2000);
	void * ptr4 = malloc(2000);
	void * ptr5 = malloc(2000);

	printf("\nMemEntry list after allocations:\n");
	PrintList();

	printf("\n");
	free(ptr1);
	free(ptr2);
	free(ptr3);
	free(ptr4);
	free(ptr5);

	printf("\nMemEntry list after all data freed:\n");
	PrintList();

	printf("\nNOTICE: Two new blocks of 5000 bytes were created for data allocation in data segment using sbrk.\n");

	return 0;
}
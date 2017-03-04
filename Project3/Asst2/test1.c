/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test1: Demonstrate merge and split of MemEntry structs (Fragmentation):\n");

	void * ptr1 = malloc(500);
	void * ptr2 = malloc(1000);
	void * ptr3 = malloc(2000);
	printf("\nMemEntry list after initial allocations:\n");
	PrintList();

	printf("\n");
	free(ptr1);
	printf("MemEntry list after free ptr1:\n");
	PrintList();
	printf("\n");
	free(ptr2);
	printf("MemEntry list after free ptr2:\n");
	PrintList();
	printf("\n");
	free(ptr3);
	printf("MemEntry list after free ptr3:\n");
	PrintList();

	printf("\nNOTICE: Two or three adjacent MemEntry structs that are freed will merge to handle fragmentation:\n");


	return 0;
}
/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	int * ptr1;
	int * ptr2;
	int * ptr3;
	char * str;

	printf("Test0: General test for normal conditions:\n");
	printf("Allocate space for 3 ints (4 bytes each) into ptr1, ptr2, and ptr3:\n");
	ptr1 = (int *) malloc(sizeof(int));
	ptr2 = (int *) malloc(sizeof(int));
	ptr3 = (int *) malloc(sizeof(int));

	*ptr1 = 1;
	*ptr2 = 250;
	*ptr3 = 125;

	printf("\nValues at pointers set to 1, 250, 125 respectively:\n");
	printf("Value at ptr1: %d\n", *ptr1);
	printf("Value at ptr2: %d\n", *ptr2);
	printf("Value at ptr3: %d\n", *ptr3);

	printf("\nFreeing ptr1, ptr2, ptr3 in that order:\n");
	free(ptr1);
	free(ptr2);
	free(ptr3);

	return 0;
}
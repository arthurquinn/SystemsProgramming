/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test7: Using mycalloc:\n");

	int i = 0;
	char * ptr = (char *) calloc(10);

	for(i = 0; i < 10; i++)
		printf("Value at ptr[%d] (address %p): %d\n", i, ptr + i, ptr[i]);

	return 0;
}
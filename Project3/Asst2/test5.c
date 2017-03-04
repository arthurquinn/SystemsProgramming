/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test5: Test redundant freeing of same pointer:\n\n");

	void * p = malloc(100);
	free(p);
	free(p);


	return 0;
}
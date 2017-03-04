/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test6: Allocating and freeing same pointer multiple times:\n");
	void * p;

	p = malloc(1000);
	free(p);

	p = malloc(1000);
	free(p);

	return 0;

}
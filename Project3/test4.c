/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{
	printf("Test4: Test freeing pointers not returned by mymalloc:\n");

	char * p = (char *) malloc(200);
	free(p + 10);

	return 0;
}
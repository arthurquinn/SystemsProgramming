/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int argc, char ** argv)
{	
	printf("Test8: Invalid sizes passed to malloc\n");

	void * p;

	p = malloc(0);
	p = malloc(10000);

	return 0;
}
/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"

int main(int agrc, char ** argv)
{
	printf("Test3: Test error when freeing a pointer that was never allocated:\n\n");

	int x;
	free(&x);

	return 0;

}
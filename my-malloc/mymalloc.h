/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 2 - Memory Allocator
 */

 #ifndef MYMALLOC
 #define MYMALLOC
 #include <stdlib.h>
 #include <stdio.h>
 #include <unistd.h>

 #define CODE 382987123
 #define CAPACITY 5000
 #define malloc(x) mymalloc((x), __FILE__, __LINE__)
 #define free(x) myfree((x), __FILE__, __LINE__)
 #define calloc(x) mycalloc((x), __FILE__, __LINE__)

/* memspace is a char array of 5000 bytes that the mymalloc library will use to allocate memory for caller functions
 */
static char memspace[5000];

/* MemEntry_ is a struct that stores information about a memory request received by mymalloc
 * describe further__________________________
 */
typedef struct MemEntry_ MemEntry;
struct MemEntry_
{
	unsigned int specialcode;
	size_t numbytes;
	int isfree;
	int arrayno;
	MemEntry * next;
	MemEntry * previous;
};

/* mymalloc takes an argument of type size_t and returns a pointer to an allocated block of memory of this size
 * located on the heap
 *
 * It also received arguments of __FILE__ and __LINE__ from its caller that will be used in the event that it needs to print an error or warning message
 * 
 */
 void * mymalloc(size_t, char *, unsigned int);

/* mycalloc takes an argument of type size_t and returns a pointer to an allocated block of memory of this size
 * located on the heap
 *
 * Also, the pointer returned by mycalloc (which is a pointer to the space requested by the user) will point to a memory segment of the requested size 
 * where all values in that memory segment are initialized to 0.
 *
 * It also received arguments of __FILE__ and __LINE__ from its caller that will be used in the event that it needs to print an error or warning message
 */
 void * mycalloc(size_t, char *, unsigned int);

/* My frees a previously allocated block of memory from memspace
 *
 * If myfree finds two (or potentially three) adjacent freed MemEntry structs, it will merge these structs into one MemEntry struct
 * for the purposes of handling data fragmentation 
 *
 * It also received arguments of __FILE__ and __LINE__ from its caller that will be used in the event that it needs to print an error or warning message
 */
 int myfree(void *, char *, unsigned int);

 #endif
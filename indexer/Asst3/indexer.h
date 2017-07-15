/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 3 - Indexer
 */

#ifndef INDEXER
#define INDEXER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include "tokenizer.h"
#include "sorted-list.h"




//defines the types Indexer and RecordList as SortedListPtr types for use in indexer
typedef SortedListPtr Index;
typedef SortedListPtr RecordList;

//defines pointers to structs WordEnt_ and RecordEnt_
typedef struct WordEnt_ * WordEnt;
typedef struct RecordEnt_ * RecordEnt;

//struct WordEnt_ will hold the word token and a list of records (files and occurences)
struct WordEnt_
{
	char * word;
	SortedListPtr recordlist;
};

//struct RecordEnt_ will hold the file a word appeared in and the number of times it occured there
struct RecordEnt_
{
	char * file;
	int occurences;
};


//CompareWordEnt is the comparator function for WordEnt_ structs that will be passed to the sorted list
//managing the index.
int CompareWordEnt(void *, void *);

//DestroyWordEnt is the destroyer function for WordEnt_ structs that will be passed to the sorted list
//managing the index.
void DestroyWordEnt(void *);

//CompareRecordEnt is the comparator function for RecordEnt_ structs that will be passed to the sorted list
//managing the RecordList
int CompareRecordEnt(void *, void *);

//DestroyRecordEnt is the destroyer function for RecordEnt_ structs that will be passed to the sorted list
//managing the RecordList
void DestroyRecordEnt(void *);

//This function accepts a char * dirname and creates and index of all files and files within its subdirectories
Index CreateIndex(char *);

//This function accepts an Index and destroys all data allocated to the heap it contains
void DestroyIndex(Index);

//This function recursively searches a directory and stores all tokens an records in the Index id
int SearchDirectory(Index, char *);

//This function writes all data stored in an Index to a given file in the proper JSON notation
int WriteIndexToFile(Index, char *);

//This is a helper function for WriteIndexToFile that manages writing RecordList sorted lists to the file
int WriteRecordListToFile(WordEnt, FILE *);

//This is a helper function that gets a new path name by appending a new directory or file name to an existing file path name
char * getnewpath(char *, char *);

//This function will insert all tokens found in a file using a TokenizerT into an Index
void InsertTokensIntoIndex(Index, TokenizerT *, char *);

//This function will create a record using a given file name to be stored in a RecordList sorted list
RecordEnt CreateRecord(char *);

//This function adds a RecordEnt struct to a RecordList sorted list
void AddRecord(RecordList, RecordEnt);

//This is a simple helper function to convert all characters in a given string to lower case
char * tolowercase(char *);

#endif
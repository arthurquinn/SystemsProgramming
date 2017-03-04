/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 3 - Indexer
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include "tokenizer.h"
#include "sorted-list.h"
#include "indexer.h"

#define DIRENT_IS_DIRECTORY direntry->d_type == DT_DIR && strcmp(direntry->d_name, ".") && strcmp(direntry->d_name, "..")
#define DIRENT_IS_FILE direntry->d_type == DT_REG
#define NEW_WORD_ENT (WordEnt) malloc(sizeof(struct WordEnt_))
#define NEW_RECORD_ENT (RecordEnt) malloc(sizeof(struct RecordEnt_))
#define EXISTS != NULL
#define DOES_NOT_EXIST == NULL


Index CreateIndex(char * dirname)
{
	//Creates the function pointers for WordEnt_ structs that will be passed to SLCreate
	int (*cf)(void *, void *) = CompareWordEnt;
	void (*df)(void *) = DestroyWordEnt;
	//Creates the Index (sorted list) data structure
	Index id = SLCreate(cf, df);
	//Searches directory, if there was an error opening the directory prints the message and returns NULL
	if(dirname == NULL) 
	{
		printf("No directory given.\n");
		return NULL;
	}
	if(!SearchDirectory(id, dirname))
	{
		printf("Directory not found. Failed to search directory.\n");
		return NULL;
	}
	//Returns the created index to the caller
	return id;
}

int CompareWordEnt(void * obj1, void * obj2)
{
	//casts the void * objects to WordEnt pointers
	WordEnt a = (WordEnt) obj1;
	WordEnt b = (WordEnt) obj2;
	//If the word token stored in a is smaller than b (alphanumerically) return -1
	//Else if the word token stored in a is greater than b (alphanumerically) return 1
	//Else return 0
	if(strcmp(a->word, b->word) > 0) return -1;
	else if(strcmp(a->word, b->word) < 0) return 1;
	else return 0;
}

void DestroyWordEnt(void * obj)
{
	//Casts a void * pointer to a WordEnt pointer
	WordEnt a = (WordEnt) obj;
	//Frees all associated data
	free(a->word);
	SLDestroy(a->recordlist);
	free(a);
}

int CompareRecordEnt(void * obj1, void * obj2)
{
	//Casts two void * pointers to RecordEnt pointers
	RecordEnt a = (RecordEnt) obj1;
	RecordEnt b = (RecordEnt) obj2;
	//If the files in both RecordEnt a and b are equal, then the RecordEnts are considered equal and returns 0
	if(!strcmp(a->file, b->file)) return 0;
	//Otherwise order the two RecordEnt_ structs by their occurences values, larger coming first
	if(a->occurences < b->occurences) return -1;
	else if(a->occurences > b->occurences) return 1;
	else
	{
		//Otherwise, if the occurences are equal, order them alphanumberically by their file name
		if(strcmp(a->file, b->file) > 0) return -1;
		else if(strcmp(a->file, b->file) < 0) return 1;
		else return 0;
	}
}

void DestroyRecordEnt(void * obj)
{
	//Casts a void * pointer to a RecordEnt pointer
	RecordEnt a = (RecordEnt) obj;
	//Frees all associated data
	free(a->file);
	free(a);
}

void DestroyIndex(Index id)
{
	//Destroys all necessary heap data associated with an index
	SLDestroy(id);
}

int SearchDirectory(Index id, char * dirname)
{
	char * newpath = NULL;
	char * token = NULL;
	struct dirent * direntry = NULL;
	TokenizerT * tk = NULL;
	//If the dirname starts with the character sequence "./" then remove them from the string
	//The character sequence is considered to be a valid dirname given the dirname that follows is also valid
	//However we remove them as they are unecessary and will make our index appear more cluttered than necessary
	if(strlen(dirname) > 2 && dirname[0] == '.' && dirname[1] == '/')
		dirname += 2;
	//The same is said for a trailing '/' character at the end of the dirname
	//This is also valid, but is removed to produce a cleaner index
	if(dirname[strlen(dirname) - 1] == '/')
		dirname[strlen(dirname) - 1] = 0;
	//Open the directory
	DIR * dirp = opendir(dirname);
	if(dirp DOES_NOT_EXIST)
	{
		//if the errno was set to ENOTDIR after our call to the opendir function
		//than it may be a file and not a directory
		//to test we will use fopen
		if(errno == ENOTDIR)
		{
			//attempt to open file
			FILE * fd = fopen(dirname, "r");
			//if fopen did not return null, then the file successfully opened and we will
			//read in all of its tokens and store them in the index
			if(fd EXISTS)
			{
				tk = TKCreate(dirname);
				InsertTokensIntoIndex(id, tk, dirname);
				TKDestroy(tk);
				//1 is returned to signal all tokens in this file were successfully inserted
				//and the index was created successfully
				return 1;
			}

		}
		//If the directory does not exist, and if errno is set to a value other than ENOTDIR, or if fopen returned null
		//print an error message represented by errno and return 0 to signal index creation failed
		printf("%s\n", strerror(errno));
		return 0;
	}
	else
	{
		//Read the direntry and continue the while loop while it exists
		direntry = readdir(dirp);
		while(direntry EXISTS)
		{
			//If it is a directory, open the directory and recursively search within
			if(DIRENT_IS_DIRECTORY)
			{
				newpath = getnewpath(dirname, direntry->d_name);
				SearchDirectory(id, newpath);
				free(newpath);
				newpath = NULL;	
			}
			//If it is a file, insert all tokens found within that file into the Index id
			else if(DIRENT_IS_FILE)
			{
				newpath = getnewpath(dirname, direntry->d_name);
				tk = TKCreate(newpath);
				InsertTokensIntoIndex(id, tk, direntry->d_name);
				TKDestroy(tk);
				free(newpath);
			}
			//Get the next direntry
			direntry = readdir(dirp);
		}
		//close the dirent handler
		closedir(dirp);
	}
	//return 1 for successful insertion of all tokens within the directory
	return 1;
}

char * getnewpath(char * dirname, char * newdir)
{
	//Allocate new memory on the heap for a new path name
	//This memory block should be of length equal to the original newname, the dirname to be appended and
	//2 additional bytes for a '/' char between them and a terminating null byte
	char * newpath = (char *) calloc(strlen(dirname) + strlen(newdir) + 2, 1);
	//append the two dirnames together with a separating '/'
	//strcat adds the terminating null byte automatically
	newpath = strcpy(newpath, dirname);
	newpath = strcat(newpath, "/");
	newpath = strcat(newpath, newdir);
	//return the resulting newpath
	return newpath;
}

void InsertTokensIntoIndex(Index id, TokenizerT * tk, char * filename)
{
	WordEnt went = NULL;
	WordEnt cmpwent = NULL;
	RecordEnt rent = NULL;
	SortedListIteratorPtr iditer = NULL;
	//Create the two function pointers that willl be passed to the RecordList that will store 
	//RecordEnt_ structs for each record for each word token
	int (*cf)(void *, void *) = CompareRecordEnt;
	void (*df)(void *) = DestroyRecordEnt;
	//store the next token found within the file into char * word
	char * word = TKGetNextToken(tk);
	//continue the loop while the returned token is not null
	while(word EXISTS)
	{
		//create a new WordEnt_ struct the the obtained token to be stored in the Index
		went = NEW_WORD_ENT;
		//Error message to be printed if malloc failed
		if(went DOES_NOT_EXIST)
		{
			printf("%s\n", strerror(errno));
			return;
		}
		//Convert the word to lower case, create an empty record list for its recordlist field
		//obtian the relative path name where the word was found and create a new record using this
		//new relative path name
		went->word = tolowercase(word);
		went->recordlist = SLCreate(cf, df);
		rent = CreateRecord(filename);
		//Attempt to insert the new WordEnt struct into the Index
		//The insert will be unsuccessful when the word is already contained within a WordEnt struct already
		//found within the Index
		//In this case, instead of adding a duplicate word, we must add a new RecordEnt_ struct to the WordEnt_ struct's RecordList
		if(!SLInsert(id, went))
		{
			iditer = SLCreateIterator(id);
			//Loop using a sorted list iterator to obtain a pointer to the WordEnt_ struct with and equivalent word token
			do
			{
				cmpwent = SLNextItem(iditer);
			} while(cmpwent EXISTS && strcmp(cmpwent->word, went->word));
			//If it was not found print an error message (this should never occur as a condition for entering this loop was that a duplicate existed)
			if(cmpwent DOES_NOT_EXIST)
			{
				printf("unsuccessful insert\n");
			}
			else
			{
				//free the WordEnt_ struct that we will no longer need and add a new record to the RecordList
				free(went);
				AddRecord(cmpwent->recordlist, rent);
			}
			//Destroy the iterator used above
			SLDestroyIterator(iditer);
		}
		//If the insert was successful this means this is the first time that word token was found and thus it received the newly created 
		//RecordEnt struct rent as the first member of its RecordList
		else 
			AddRecord(went->recordlist, rent);
		rent = NULL;
		went = NULL;
		//Get the next token and continue iteration until this function returns NULL
		word = TKGetNextToken(tk);
	}
}

char * tolowercase(char * word)
{
	//Simple code to convert all characters in a given string to lower case (if applicable)
	int i = 0;
	for(i; i < strlen(word); i++)
	{
		word[i] = tolower(word[i]);
	}
	return word;
}

void AddRecord(RecordList recordlist, RecordEnt rent)
{
	RecordEnt temp = NULL;
	SortedListIteratorPtr iter = NULL;
	//Attempts to insert a RecordEnt_ struct into a RecordList
	//If the RecordEnt does not already exist within the RecordList, the RecordEnt is added
	//and the function exits
	//If the RecordEnt is found to be a duplicate, the if block executes
	if(!SLInsert(recordlist, rent))
	{
		//Create an iterator to traverse the RecordList until a dupilicate RecordEnt is found
		iter = SLCreateIterator(recordlist);
		do
		{
			temp = SLNextItem(iter);
		} while(temp EXISTS && strcmp(temp->file, rent->file));
		//Destroy the iterator as it is no longer needed
		SLDestroyIterator(iter);
		if(temp DOES_NOT_EXIST)
		{
			//this case should never happen as the list is only searched if we know temp exists.
			printf("unsuccessful insert\n"); //add more
		}
		else
		{
			//Take the occurence field of the duplicate RecordEnt within the RecordList, increment it, and store this
			//value in the new RecordEnt that will be stored in the RecordList
			rent->occurences = temp->occurences + 1;
			//Remove the old RecordEnt and insert our new one to ensure the RecordList is kept in proper order
			SLRemove(recordlist, temp);
			SLInsert(recordlist, rent);
		}
	}

}

RecordEnt CreateRecord(char * file)
{
	//Allocate memory for a new RecordEnt struct
	RecordEnt rent = NEW_RECORD_ENT;
	//If malloc failed print an error and return NULL
	if(rent DOES_NOT_EXIST)
	{
		printf("%s\n", strerror(errno));
		return NULL;
	}
	//Allocate space to store the name of the file within the RecordEnt struct
	char * fileobj = (char *) calloc(strlen(file) + 1, 1);
	//If calloc failed print error message and return NULL
	if(fileobj DOES_NOT_EXIST)
	{
		printf("%s\n", strerror(errno));
		return NULL;
	}
	//Copy the file string into the newly created fileobj and store this, along with occurences initialized
	//to 1 into the new RecordEnt struct
	fileobj = strcpy(fileobj, file);
	rent->file = fileobj;
	rent->occurences = 1;
	//Return the newly created RecordEnt
	return rent;
}

int WriteIndexToFile(Index id, char * outfile)
{
	char c = 0;
	//Uses the access function to determine if the output file specified by the user
	//already exists and if it does whether or not the user wants to overwrite it
	if(!access(outfile, F_OK))
	{
		//Alert the user that the file already exists and determine if they would like to overwrite
		//or leave the file as it is
		printf("File \"%s\" already exists. Overwrite? y/n\n", outfile);
		do
		{
			c = getchar();
		} while(!(c == 'y' || c == 'n'));
		if(c == 'n')
		{
			//If they do not want to overwrite the file print this message and 
			//return 0 to indicate an error writing the index to the file
			printf("File not overwritten. Execute command again with valid file name.\n");
			return 0;
		}
		//Otherwise if they want to overwrite the file, continue as normally
		else
			printf("Overwriting file: %s\n", outfile);
	}
	//Attempts to open the output file for writing
	FILE * out = fopen(outfile, "w");
	//If the file was not opened print the error indicated by errno and the following message
	if(out == NULL)
	{
		printf("%s\nCould not open or create output file. Enter valid file name.\n", strerror(errno));
		return 0;
	}
	//Create an iterator to traverse the index
	SortedListIteratorPtr iditer = SLCreateIterator(id);
	WordEnt went = SLNextItem(iditer);
	//Print out the initial header of the index representation
	if(fprintf(out, "{\"list\":[\n") < 0) return 0;
	//For every WordEnt found within the Index, do the following
	while(went EXISTS)
	{
		//First print out the word token followed by the RecordList found within
		if(fprintf(out, "\t{\"%s\":[\n", went->word) < 0) return 0;
		if(!WriteRecordListToFile(went, out)) return 0;
		//Get the next item
		went = SLNextItem(iditer);
		//If the next item exists, print a comma after the closing braces
		if(went EXISTS)
		{
			if(fprintf(out, "\t]},\n") < 0) return 0;
		}
		//Otherwise, no comma is necessary
		else
		{
			if(fprintf(out, "\t]}\n") < 0) return 0;
		}
	}
	//Print the final closing braces, detroy the iterator and close the file
	if(fprintf(out, "]}\n") < 0) return 0;
	SLDestroyIterator(iditer);
	fclose(out);
	//Return 1 to signal the index was successfully printed to the file.
	return 1;
}

int WriteRecordListToFile(WordEnt went, FILE * out)
{
	//Create a sorted list iterator to iterator through the record list stored within a WordEnt
	SortedListIteratorPtr rliter = SLCreateIterator(went->recordlist);
	RecordEnt rent = SLNextItem(rliter);
	//For every RecordEnt that exists within the list, do the following
	while(rent EXISTS)
	{
		//Print out the record entry in the appropriate format
		if(fprintf(out, "\t\t{\"%s\":%d}", rent->file, rent->occurences) < 0) return 0;
		rent = SLNextItem(rliter);
		//IF the next entry exists, print closing braces followed by a comma
		if(rent EXISTS)
		{
			if(fprintf(out, ",\n") < 0) return 0;
		}
		//Otherwise no comma is necessary
		else
		{
			if(fprintf(out, "\n") < 0) return 0;
		}
	}
	//Destroy the iterator and return 1 to signal successful printing
	SLDestroyIterator(rliter);
	return 1;
}



int main(int argc, char ** argv)
{
	//Creates an index using the second argument provided by the user
	Index id = CreateIndex(argv[2]);

	//IF the index was not created for whatever reason, print the following error message
	//and exist with EXIT_FAILURE
	if(id DOES_NOT_EXIST)
	{
		printf("Error creating index.\n");
		exit(EXIT_FAILURE);
	}

	//If there was an error writing to the specified file, print the message, destroy the index
	//and exit with EXIT_FAILURE
	if(!WriteIndexToFile(id, argv[1]))
	{
		printf("Error writing index to file.\n");
		DestroyIndex(id);
		exit(EXIT_FAILURE);
	}

	//Otherwise, print a success message to the terminal
	printf("Successfully wrote index to file: %s\n", argv[1]);

	//Destroy the index and exit with EXIT_SUCCESS
	DestroyIndex(id);
	exit(EXIT_SUCCESS);
}




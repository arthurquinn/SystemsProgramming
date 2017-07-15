/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 3 - Indexer
 */

/*
 * tokenizer.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "tokenizer.h"


TokenizerT * TKCreate(char * filename)
{
	TokenizerT * tk = (TokenizerT *) malloc(sizeof(TokenizerT));
	if(tk == NULL) return NULL;

	FILE * fd = fopen(filename, "r");
	char c;
	int i = 1;

	tk->filetext = malloc(i);
	c = fgetc(fd);
	while(c != EOF)
	{
		tk->filetext[i - 1] = c;
		tk->filetext = (char *) realloc(tk->filetext, ++i);
		c = fgetc(fd);
	}
	tk->filetext[i - 1] = '\0';
	tk->pos = 0;
	return tk;
}

void TKDestroy(TokenizerT * tk)
{
	//the dynamically allocated memory is freed and the pointer is set to null
	free(tk->filetext);
	free(tk);
	tk = NULL;
}

char * TKGetNextToken(TokenizerT * tk)
{
	char * tok = (char *) calloc(strlen(tk->filetext) - tk->pos + 1, 1);
	if (tok == NULL) 
	{
		printf("Failed to allocate memory for the return token\n");
		return NULL; //if calloc failed, an error message is printed and TKGetNextToken returns null
	}
	do
	{
		if(isalpha(tk->filetext[tk->pos])) 
			return checkWord(tk, tok);
	} while(tk->filetext[tk->pos++] != '\0');
	free(tok);
	return 0;
}

char * checkWord(TokenizerT * tk, char * tok)
{
	int i = 0;
	do
	{
		tok[i++] = tk->filetext[tk->pos++];	
	} while(isalpha(tk->filetext[tk->pos]) || isdigit(tk->filetext[tk->pos]));
	return (char *) realloc(tok, strlen(tok) + 1);
}


/* Arthur Quintanilla and Razeno Khan
 * Systems Programming
 * Assignment 3 - Indexer
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#ifndef TOKENIZER_H
#define TOKENIZER_H

struct TokenizerT_
{
	char * filetext;
	int pos;
};

typedef struct TokenizerT_ TokenizerT;

char * checkWord(TokenizerT *, char *);

TokenizerT * TKCreate(char *);

void TKDestroy(TokenizerT *);

char * TKGetNextToken(TokenizerT *);

char * checkWord(TokenizerT *, char *);

#endif
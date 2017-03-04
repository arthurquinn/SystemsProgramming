/*
 * tokenizer.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*
 * Tokenizer type.  You need to fill in the type as part of your implementation.
 */

/* The TokenizerT struct will store a(n):
 * char pointer, which upon initializaiton will point to (but never modify) argv[1]
 * integer pos, which will represent the current position within the token stream for use 
 *   by TKGetNextToken the functions it calls
 * char pointer type, to store a string literal type that denotes the type of token that was returned
 */
struct TokenizerT_
{
	char * input;
	int pos;
	char * type;
};

typedef struct TokenizerT_ TokenizerT;

char * checkWord(TokenizerT *, char *);
char * checkHex(TokenizerT *, char *);
char * checkOctal(TokenizerT *, char *);
char * checkDecimal(TokenizerT *, char *);
char * checkFloat(TokenizerT *, char *);
char * checkOperator(TokenizerT *, char *);
char * checkComment(TokenizerT *, char *);
char * checkQuotes(TokenizerT *, char *);
int isInOpArray(char *, int);
int posInOpArray(char *, int);
int isInKeyArray(char *, int);

const char * OPERATORS[] = {"(", ")", "[", "]", ".", "->", "&", "!", "~", "++",
						  "--", "*", "/", "%", "+", "-", ">>", "<<", "<", ">", "<=",
						  ">=", "==", "!=", "^", "|", "&&", "||", "?", ":", "=", "+=",
						  "-=", "*=", "/=", "%=", ">>=", "<<=", "&=", "^=", "|=", ",", "//", "/*", "*/", "\"", "'"};

const char * OPERATOR_NAMES[] = {"Left Parenthesis", "Right Parenthesis", "Left Brace", "Right Brace", "Dot", "Structure Pointer", "Address",
								 "Negate", "1's Comp", "Plus Plus", "Minus Minus", "Multiply", "Divide", "Modulus", "Add", "Subtract", "Shift Right",
								 "Shift Left", "Less Than", "Greater Than", "Less Than Equals", "Greater Than Equals", "Equals Equals", "Not Equals",
								 "Bitwise Exclusive Or", "Bitwise Or", "Logical And", "Logical Or", "Ternary", "Condition", "Equals", "Plus Equals",
								 "Minus Equals", "Multiply Equals", "Divide Equals", "Modulus Equals", "Shift Right Equals", "Shift Left Equals", "Address Equals",
								 "Bitwise Exclusive Or Equals", "Bitwise Or Equals", "Comma", "Single Comment", "Comment Start", "Comment End", "Double Quote", "Single Quote"};

const char * KEYWORDS[] = {"auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern",
						   "float", "for", "goto", "if", "int", "long", "register", "return", "short", "signed", "sizeof", "static",
						   "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"};

const int OPERATORS_SIZE = 47;
const int KEYWORDS_SIZE = 32;

/*
 * TKCreate creates a new TokenizerT object for a given token stream
 * (given as a string).
 *
 * TKCreate should copy the arguments so that it is not dependent on
 * them staying immutable after returning.  (In the future, this may change
 * to increase efficiency.)
 *
 * If the function succeeds, it returns a non-NULL TokenizerT.
 * Else it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */
TokenizerT * TKCreate(char * ts)
{
	//dynamically allocates enough memory for a TokenizerT, if malloc fails NULL is returned
	//and the error is printed from main
	//all other values are initialized and the pointer to the TokenizerT struct is returned
	TokenizerT * tk = (TokenizerT *) malloc(sizeof(TokenizerT));
	if(tk == NULL) return NULL;
	tk->input = ts;
	tk->pos = 0;
	tk->type = "";
	return tk;
}

/*
 * TKDestroy destroys a TokenizerT object.  It should free all dynamically
 * allocated memory that is part of the object being destroyed.
 *
 * You need to fill in this function as part of your implementation.
 */
void TKDestroy(TokenizerT * tk)
{
	//the dynamically allocated memory is freed and the pointer is set to null
	free(tk);
	tk = NULL;
}

/*
 * TKGetNextToken returns the next token from the token stream as a
 * character string.  Space for the returned token should be dynamically
 * allocated.  The caller is responsible for freeing the space once it is
 * no longer needed.
 *
 * If the function succeeds, it returns a C string (delimited by '\0')
 * containing the token.  Else it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

/* TKGetNextToken is responsible for analyzing and returning the next token in the token stream starting at the position 
 * denoted by the value stored in tk->pos. 
 *
 * The next token can be of the type Word, Decimal, Float, Hex, Octal, Operator, Comment, or Quote. Floats are treated 
 * as a specific form of decimal, and comments/quotes are treated as a specific form of an operator.
 *
 * If the first character is a letter, TKGetNextToken will call checkWord to analyze the next complete word token. If 
 * this word token is found to be a keyword specified the KEYWORDS array the type field of the TokenizerT will be set to 
 * Keyword.
 *
 * If the first character is a digit of value 1 through 9 the checkDecimal function will be called to analyze the next 
 * complete decimal token. If a '.' is found in the stream immediately following a digit 0 through 9 the checkFloat function 
 * will be called. The digit '0' was omitted here because they are a special case described below.
 * 
 * If the first character is a '0' there are three possibilities 
 * One: The next character is an 'x' or 'X' which signifies a hexadecimal token to be analyzed.
 * Two: The next character is a value of 0 through 7 which signifies an octal digit to be analyzed. 
 * Three: The next character is neither of the above. In this case we have a 0 followed be essentially anything except
 *		  for what we defined above. This was chosen to be interpreted as a decimal. This is beneficial in all cases. For
 *        example: 0 will return decimal value 0; 0.0 will return float value 0.0; 0. will return decimal 0 followed by 
 *        dot operator '.'; and most importantly, it allows octals to have as many leading 0's as they wish. Decimals
 *        on the otherhand will be denied leading 0's given this design choice, but we felt it was a worthy trade off
 *        as, from a programming standpoint, leading 0's to octals are more important than leading 0's to decimals. The
 *        only time a decimal needs a leading 0 is when it is a zero itself, or is meant to start a float (Ex. 0.23).
 *
 * If the first character is none of the above, and also not white space, it will be passed to the checkOperator function
 * to see if it is an operator described in the OPERATORS array.
 * 
 * If the character satisfies none of the above conditions it must be white space or a null terminator. If it is white
 * space (that is not the null temrinator) the position field of the tokenizer struct is incremented and the loop executes 
 * again (effectively "passing over" all instances of white space). If it is a null terminator, the dynamically allocated memory
 * is freed and the value 0 is returned which will signal to main that the end of the token stream has been reached. 
 */
char * TKGetNextToken(TokenizerT * tk)
{
	char * tok = (char *) calloc(strlen(tk->input) - tk->pos + 1, 1);
	if (tok == NULL) 
	{
		printf("Failed to allocate memory for the return token\n");
		return NULL; //if calloc failed, an error message is printed and TKGetNextToken returns null
	}
	do
	{
		if(isalpha(tk->input[tk->pos])) return checkWord(tk, tok);
		else if(isdigit(tk->input[tk->pos]) && tk->input[tk->pos] != '0') return checkDecimal(tk, tok);
		else if(tk->input[tk->pos] == '0')
		{
			if(tk->input[tk->pos + 1] == 'x' || tk->input[tk->pos + 1] == 'X') return checkHex(tk, tok);
			else if(tk->input[tk->pos + 1] >= 48 && tk->input[tk->pos + 1] <= 55) return checkOctal(tk, tok);
			else return checkDecimal(tk, tok);
		}
		else if(!isspace(tk->input[tk->pos]) && tk->input[tk->pos] != '\0') return checkOperator(tk, tok);
	} while(tk->input[tk->pos++] != '\0');
	free(tok);
	return 0;
}

/* This function is fairly straightforward as it simply assigns the character in position tk->pos
 * of the input string to be analyzed to its corresponding position in tok (which was determined to be 
 * an alphabetic character in the caller) and repeats this process for each subsequent character that is
 * determined to be either a digit or another alphabetic character.
 * After this process is completed it checks if the retrieved token is in the constant KEYWORDS array defined above
 * If it is in the array the type is set to Keyword, else it is set to Word
 * The token is then reallocated to its appropriate size and returned
 * The size of the reallocation is the length of token plus one position for a null terminator
 */
char * checkWord(TokenizerT * tk, char * tok)
{
	int i = 0;
	do
	{
		tok[i++] = tk->input[tk->pos++];	
	} while(isalpha(tk->input[tk->pos]) || isdigit(tk->input[tk->pos]));
	tk->type = isInKeyArray(tok, 0) ? "Keyword" : "Word";
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* This function assigns each character at position tk->pos of the inputted string that is determined to
 * be a digit to its corresponding position in tok and sets the type stored in the TokenizerT struct to Decimal
 * After a character that is not a digit is found in the input string, it determines if that character is a '.'
 * If it is, the checkFloat function is called
 * Otherwise the token is then reallocated to an appropriate size in memory and returned
 * The size of the reallocation is the length of token plus one position for a null terminator
 */
char * checkDecimal(TokenizerT * tk, char * tok)
{
	int i = 0;
	do
	{
		tok[i++] = tk->input[tk->pos++];
	} while(isdigit(tk->input[tk->pos]));
	tk->type = "Decimal";
	if(tk->input[tk->pos] == '.') return checkFloat(tk, tok);
	else return (char *) realloc(tok, strlen(tok) + 1);
} 

/* This function finds and returns the next float token stored within the inputted token stream starting at the end of the previous decimal
 * and continuing until the first non-digit character following the '.' (with special exemptions for the first 'e' character which may represent
 * exponential notation). Since the checkFloat function is only ever called from checkDecimal the position will always be at the '.' following a decimal
 * and the passed char * tok will always contian a decimal number to which the ending portion of the float will be appended in this function.
 * 
 * This function accepts a TokenizerT * and a char *. The char * tok will be any string of 
 * numeric characters. For this reason the iterative variable i is not set to 0, but rather to the 
 * length of the token to begin insertion of subsequent characters after these initial ones (the initial ones being the previous decimal found
 * within the checkDecimal function).
 * 
 * The current position within the inputted string, tk->pos, will always be at the decimal point, '.', since this is
 * the only case in which this function will be called.
 *
 * If there is no digit following this decimal point, the token is returned in the same state that it was passed
 * Since this function is only called from checkDecimal the same decimal value will be returned to checkDecimal where it
 * will be returned to TKGetNextToken as a decimal type. Upon the next iteration of TKGetNextToken, the decimal point will be interpreted 
 * as Dot operator, rather than a decimal point here (since values such as 123. are not considered to be floats) (Rather, it was decided that
 * the best course of action for these "dangling" decimal point was to treat them as instances of the dot operator).
 *
 * If, however, the decimal point is followed by a digit this will be read into tok as will all subsequent numeric characters.
 * When a non-numeric character is discovered iteration will end and the float token will be realloc'd to an appropriate size and returned
 *
 * There is one important exception to this rule. If an 'e' is found at least one digit after the decimal point, and is followed by at least one digit,
 * and has not been found before for this specific token, then this 'e' character will be treated as exponentional notation. 
 * There are two cases for exponential notation. The first case is the 'e' followed by at least one digit and the second is 'e' followed by '-' followed
 * by at least one digit. These two cases are mutually exclusive and either one of them can occur only once
 *
 * The float is then reallocated to an appropriate size and returned.
 */
char * checkFloat(TokenizerT * tk, char * tok)
{
	int i = (int) strlen(tok);
	int eflag = 0;
	if(!isdigit(tk->input[tk->pos + 1])) return (char *) realloc(tok, strlen(tok) + 1);
	do
	{
		tok[i++] = tk->input[tk->pos++];
		if(tk->input[tk->pos] == 'e' && tk->input[tk->pos + 1] == '-' && isdigit(tk->input[tk->pos + 2]) && !eflag)
		{
			tok[i++] = 'e';
			tok[i++] = '-';
			tk->pos += 2;
			eflag = 1;
		}
		else if(tk->input[tk->pos] == 'e' && isdigit(tk->input[tk->pos + 1]) && !eflag)
		{
			tok[i++] = 'e';
			tk->pos++;
			eflag = 1;
		}
	} while(isdigit(tk->input[tk->pos]));
	tk->type = "Float";
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* This function finds and returns the next hexadecimal token stored within the inputted token stream starting at tk->pos
 * and ending with the first instance of a non-digit or a character that is not A through F or a through f following a sequence of either
 * '0' and 'x' or '0' and 'X'
 *
 * When a '0' followed by an 'x' or an 'X' is found in TKGetNextToken, this function, checkHex, is called the first two of those
 * characters are then stored in tok. Then, if the next character is a digit, a through f, or A through F that character is stored in its 
 * corresponnding position within tok iteratively until a character that does not satisfy the previous conditions is found
 *
 * After iteration, if the length of the stored token is less than three (that is "0x" or "OX") this is considered to be a bad hexadecimal token
 * and, although the bad token is returned, it is returned with type "Bad Token". If the length of the token is 3 or more (that is "0x" or "0X" followed
 * by at least one digit or a through f or A through F then its type is set to "Hexadecimal" and returned.
 *
 * Upon either of these cases the returned token is reallocated to its appropriate size and returned
 */
char * checkHex(TokenizerT * tk, char * tok)
{
	int i = 0;
	tok[i++] = tk->input[tk->pos++];
	tok[i++] = tk->input[tk->pos++]; //this line and the above line will find the '0' and either 'x' or 'X' characters and place them in their proper positions within tok
	while(isdigit(tk->input[tk->pos]) || (tk->input[tk->pos] >= 65 && tk->input[tk->pos] <= 70) || (tk->input[tk->pos] >= 97 && tk->input[tk->pos] <= 102))
		tok[i++] = tk->input[tk->pos++];
	tk->type = strlen(tok) < 3 ? "Bad Token" : "Hexadecimal";
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* This function finds and returns the next octal token stored within the inputted token stream starting at tk->pos and 
 * ending with the first instance of a character that is not a digit of value '0' through '7'.
 * 
 * Since it was determined in TKGetNextToken that the inputted token stream, starting at tk->pos, contains a '0' followed by at least
 * one more digit of value '0' through '7' we can safely assume within this function that the octal token will in no case be a bad token. Therefore,
 * we can also safely assume the below while loop will execute at least twice, resulting in an octal token of length 2 or more.
 *
 * Iteration of this loop will continue until the first value that is not a digit between and including '0' through '7'
 * The type stored within the TokenizerT struct will then be set to "Octal" and the octal token will be reallocated  
 * to its appropriate size and returned.
 */
char * checkOctal(TokenizerT * tk, char * tok)
{
	int i = 0;
	while(tk->input[tk->pos] >= 48 && tk->input[tk->pos] <= 55)
		tok[i++] = tk->input[tk->pos++];
	tk->type = "Octal";
	return (char *) realloc(tok, strlen(tok) + 1);
}


/* This function finds and returns the next operator token stored within the inputted token stream starting at tk->pos and ending when a sequence of
 * characters not stored within the OPERATORS array is found.
 *
 * The type stored within the TokenizerT struct is intialized to "Bad Token" so if the character at tk->pos is not itself an operator or the starting character of
 * any other operator it can be immediately returned with this type.
 *
 * If, however, the character at tk->pos is found to be an operator or the start of an operator iteration can begin to determine the next operator within the
 * inputted token stream of greatest length (this means if "+=" is found in the token stream it will be returned as a plus-equals operator "+=", rather than
 * a plus operator "+" and a subsequent equals operator "=")
 * 
 * tk->pos is purposefully left un-incremented in the third statement of this function for the purpose of leaving it one position behind the integer i during the iteration
 * of the do-while loop (should it be reached) (if the do-while loop was not reached as a result of the first character of the token not being an operator stored within the
 * OPERATORS array tk->pos is incremented before this bad token is returned)
 *
 * The reason tk->pos must remain one value less than the value of i during iteration of the do-while loop is a direct result of the algorithm with which this loop was implemented
 * As it is the purpose of this function to find the longest possible operator, the loop ceases iteration at the longest possible sequence of characters that is not within
 * the OPERATORS array. It then removes the last character of the token by placing a null terminator at the first bad character. For example if the sequence of characters 
 * "->-" is found within the token stream, after iteration of the do-while loop, tok will store the first "bad" operator, that is "->-". It then will be ammended in the following
 * statement to "->" and tk->pos will remain on the second "-" which will be the starting position of the next call of TKGetNextToken
 *
 * Once the longest possible operator is found, if it is a "//" or an "/\*" the function check comment is returned to check for the entire comment
 * If it is a "\"" or an "'" checkQuotes is found to return the quoted statement
 * If neither of these cases are true, its type is set to its corresponding name found within the OPERATOR_NAMES array and the token is reallocated to its appropriate size and
 * returned.
 *
 * It is important to note that in order for an operator to receive its proper name, both the operator and operator name must be stored in the same position within their
 * respective OPERATORS and OPERATOR_NAMES arrays. 
 */
char * checkOperator(TokenizerT * tk, char * tok)
{
	int i = 0;
	tk->type = "Bad Token";
	tok[i++] = tk->input[tk->pos];
	if(!isInOpArray(tok, 0))
	{
		tk->pos++;
		return (char *) realloc(tok, strlen(tok) + 1);
	}
	do
	{
		tok[i++] = tk->input[++tk->pos];
	} while(isInOpArray(tok, 0) && tk->input[tk->pos] != '\0');
	tok[i - 1] = '\0';
	if(!strcmp(tok, "//") || !strcmp(tok, "/*")) return checkComment(tk, tok);
	if(!strcmp(tok, "\"") || !strcmp(tok, "'")) return checkQuotes(tk, tok);
	tk->type = (char *) OPERATOR_NAMES[posInOpArray(tok, 0)];
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* The checkComment function recognizes the two types of C comments, single line comments, and block comments
 *
 * Single line comments are comments that start with "//" and end with the first instance of a new line character, '\n'.
 * If a null terminator is found before the first instance of a new line character, this is still considered to be an "OK" token 
 * and the rest of the token stream is interpreted to be a single line comment.
 *
 * Block comments are comments that start with the "/\*" and end with "*\/" (the escape character is only used here so that this comment itself
 * within this C file does not end). If a null terminator is found before the first instance of "*\/", this is still considered to be an
 * "OK" token and the rest of the token stream is interpreted to be a block comment.
 *
 * After either of these terminating conditions, or a null terminator, is met, each token is assigned within the TokenizerT struct its appropriate
 * type, reallocated to an appropriate size and returned.
 */
char * checkComment(TokenizerT * tk, char * tok)
{
	int i = strlen(tok);
	if(!strcmp(tok, "//"))
	{
		while(tk->input[tk->pos] != '\n' && tk->input[tk->pos] != '\0')
			tok[i++] = tk->input[tk->pos++];
		tk->type = "Single Line Comment";
	}
	else if(!strcmp(tok, "/*"))
	{
		while(!(tok[i - 2] == '*' && tok[i - 1] == '/') && tk->input[tk->pos] != '\0')
			tok[i++] = tk->input[tk->pos++];
		tk->type = "Block Comment";
	}
	else tk->type = "Bad Token";  //else statement should never execute...a tok that is neither "//" or "/*" should never be passed
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* The checkQuotes function finds and returns an entire block of either a double-quoted or single-quoted string token.
 * 
 * If the passed tok is determined to be a double-quote character "\"" it continues iteration until the first instance of another 
 * double-quote character. This will be considered the end of the double-quoted string token. Upon each iteration all contents within both of 
 * these double quotes within the tok stream is stored in their respective positions within tok.
 *
 * If the passed tok is determined to be a single-quote character "'" it continues iteration until the first instance of another
 * single-quote character. This will be considered the end of the single-quoted string token. Upon each iteration all contents within both of 
 * these single quotes within the tok stream is stored in their respective positions within tok.
 * 
 * In both of these cases, if the current position within the token stream is not a null terminator (that is the quoted text was terminated by 
 * a " or a ') its type is set to either "Double Quoted String" or "Single Quoted String" respectively. 
 *
 * If however, the current position within the token stream is a null terminator, this implies that the double or single quoted string was not 
 * terminated properly by a corresponding double or single quote and is therefore a bad token. We do not need to set the type stored within
 * the TokenizerT struct to "Bad Token" as the type field was initialized with this value in the caller function checkOperator
 *
 * The returned tok is then reallocated to its appropriate size and returned
 */
char * checkQuotes(TokenizerT * tk, char * tok)
{
	int i = strlen(tok);
	if(!strcmp(tok, "\""))
	{
		while(!(tk->input[tk->pos - 1] == '\"' && i != 1) && tk->input[tk->pos] != '\0')
			tok[i++] = tk->input[tk->pos++];
		if (tok[i - 1] == '"') tk->type = "Double Quoted String";
	}
	else if(!strcmp(tok, "'"))
	{
		while(!(tk->input[tk->pos - 1] == '\'' && i != 1) && tk->input[tk->pos] != '\0')
			tok[i++] = tk->input[tk->pos++];
		if(tok[i - 1] == '\'') tk->type = "Single Quoted String";
	}
	//if there was no terminating " or ' the quoted text has no closure and is considered a Bad Token (this was assigned as default in checkOperators)
	return (char *) realloc(tok, strlen(tok) + 1);
}

/* This is a recursive function to determine whether the argument char * tok is stored within the global constant OPERATORS[]
 * The argument i determines the first position to start the search and it continues until it either
 * finds a match within the OPERATORS array or has reached the last position within the array, as denoted by the 
 * global constant OPERATORS_SIZE
 *
 * 1 is returned if a match was found, 0 otherwise
 */ 
int isInOpArray(char * tok, int i)
{
	if(!strcmp(tok, OPERATORS[i])) return 1;
	else if(i < OPERATORS_SIZE - 1) return isInOpArray(tok, ++i);
	else return 0;
}

/* This is a recursive function to determine at what position in the global constant OPERATORS[] the argument char * tok 
 * is stored, or if it is not stored in the array. The argument i determines the first position within the array to 
 * start the search and it continues until it either finds a match or reaches the last position within the array as 
 * denoted by the global constant OPERATORS_SIZE
 * 
 * The position tok is stored within the array is returned if it finds a match, if not it returns -1, signifying that it was not found
 * The value -1 should never be returned as this function should only be called within this program when we know that char * tok is 
 * within the OPERATORS array.
 */
int posInOpArray(char * tok, int i)
{
	if(!strcmp(tok, OPERATORS[i])) return i;
	else if(i < OPERATORS_SIZE - 1) return posInOpArray(tok, ++i);
	else return -1;
}

/* This is a recursive function to determine whether the argument char * tok is stored within the global constant KEYWORDS[]
 * The argument i determines the first position to start the search and it continues until it either
 * finds a match within the KEYWORDS array or has reached the last position within the array, as denoted by the 
 * global constant KEYWORDS_SIZE
 *
 * 1 is returned if a match was found, 0 otherwise
 */ 
int isInKeyArray(char * tok, int i)
{
	if(!strcmp(tok, KEYWORDS[i])) return 1;
	else if(i < KEYWORDS_SIZE - 1) return isInKeyArray(tok, ++i);
	else return 0;
}

/*
 * main will have a string argument (in argv[1]).
 * The string argument contains the tokens.
 * Print out the tokens in the second string in left-to-right order.
 * Each token should be printed on a separate line.
 */

 /* This programs main function will use a TokenizerT struct and functions that utilize this struct to identify and print out
  * each token stored within the first argument to main (argv[1]). Tokens will be printed out as they are identified in left to 
  * right order within the token stream. The tokens will print out in a numbered list to the right of their corresponding token type.
  *
  * If main is not called with its corrects number of arguments an error message will print and the process will terminate
  *
  * If the system is unable to malloc memory for the TokenizerT struct an error message will print and the process will terminate
  *
  * Main will print out the next token in the stream as it is identified as long as TKGetNextToken keeps finding new tokens within the 
  * stream. Once TKGetNextToken returns a 0, which signifies that a null terminator has been reached within the stream, a message will
  * be printed stating that the end of the token stream has been reached and that the process will now exit.
  * 
  * All dynamically allocated memory is freed
  * Memory allocated for the TokenizerT struct is freed in TKDestroy, which is called directly prior to the end of main, after all iterations
  * of TKGetNextToken
  * Memory allocated for the token returned by TKGetNextToken is freed after each iteration of TKGetNextToken. If howevever a null byte was reached, 
  * and TKGetNextToken returned a value of 0 rather than the dynamically allocated memory pointed to by tok, that dynamically allocated memory
  * is freed within the TKGetNextToken function itself.
  */
int main(int argc, char ** argv)
{
	if(argc != 2)
	{
		printf("Invalid number of arguments given to main\nRequires 1 string argument\nExiting...\n\n");
		return 0;
	}
	TokenizerT * thistk = TKCreate(argv[1]);
	if(thistk == NULL)
	{
		printf("Failed to allocate memory for TokenizerT...\nExiting...\n");
		return 0;
	}
	char * nextToken = NULL;
	int i = 0;
	do
	{
		nextToken = TKGetNextToken(thistk);
		if(nextToken != 0)
		{
			printf("%d: %s: %s\n", ++i, thistk->type, nextToken);
			free(nextToken);
		}
	} while(nextToken != 0);
	printf("End of token stream reached\nExiting...\n\n");
	TKDestroy(thistk);
	return 0;
}
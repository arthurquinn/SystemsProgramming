/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#ifndef SESSIONCOMMANDS_H
#define SESSIONCOMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include "server.h"


//opens an account in the bank structure
ActiveAccountInfo openaccount(char * accountname, int clientfd);

//starts an account by placing an account in the client session's ActiveAccount pointer
ActiveAccountInfo startaccount(char * accountname, int clientfd);

//credits a specified amount to an account
ActiveAccountInfo creditamount(float amount, ActiveAccountInfo ActiveAccount, int clientfd);

//debits a specified amount from an account
ActiveAccountInfo debitamount(float amount, ActiveAccountInfo ActiveAccount, int clientfd);

//retrieves the balance of an account
ActiveAccountInfo balance(ActiveAccountInfo ActiveAccount, int clientfd);

//finishes the session of an account currently in session
ActiveAccountInfo finish(ActiveAccountInfo ActiveAccount, int clientfd);

//each of the following validate the data that will be passed to their respective command functions
int ValidateOpen(int clientfd, char * accountname, ActiveAccountInfo ActiveAccount);
int ValidateStart(int clientfd, char * accountname, ActiveAccountInfo ActiveAccount);
int ValidateCredit(int clientfd, char * amountarg, float * amount, ActiveAccountInfo ActiveAccount);
int ValidateDebit(int clientfd, char * amountarg, float * amount, ActiveAccountInfo ActiveAccount);
int ValidateBalance(int clientfd, ActiveAccountInfo ActiveAccount);
int ValidateFinish(int clientfd, ActiveAccountInfo ActiveAccount);


#endif
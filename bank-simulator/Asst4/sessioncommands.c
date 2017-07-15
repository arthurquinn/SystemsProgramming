/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#include "sessioncommands.h"

//opens a new account in the Bank structure
ActiveAccountInfo openaccount(char * accountname, int clientfd)
{
	char buffer[256];
	int i = 0;

	//error if the account name is greater than 100 characters
	if(strlen(accountname) > 100)
	{
		strcpy(buffer, "Unable to create accout: Account name too long. Max size: 100 characters.\n");
		WriteToClient(clientfd, buffer);
		return NULL;
	}

	//lock the lockbank mutex so no other threads can modify shared
	//bank structure while this thread attempts to modify it
	pthread_mutex_lock(&mutex_bank);

	//wait on the sem_bank semaphore to delay server state printout if account is currently being created


	//first check if account name already exists
	for(i; i < MAX_ACCOUNTS; i++)
	{
		//if the name exists, unlock mutex and send error message to client
		if(!strcmp(Bank[i].account->name, accountname))
		{
			pthread_mutex_unlock(&mutex_bank);
			sprintf(buffer, "Unable to create accout: Account name \"%s\" already exists.\n", accountname);
			WriteToClient(clientfd, buffer);
			return NULL;
		}
	}

	//then check if there are any available positions
	for(i = 0; i < MAX_ACCOUNTS; i++)
	{
		//if the bank entry is not taken
		if(!Bank[i].taken)
		{
			//store the new account in the bank entry, unlock mutex and send success message to client
			strcpy(Bank[i].account->name, accountname);
			Bank[i].taken = 1;
			pthread_mutex_unlock(&mutex_bank);
			strcpy(buffer, "Account successfully created.\n");
			WriteToClient(clientfd, buffer);
			return NULL;
		}
	}

	//unlock bank structure for others threads to use
	pthread_mutex_unlock(&mutex_bank);

	//else there was no room
	strcpy(buffer, "Unable to create account: Account limit reached: 20\n");
	WriteToClient(clientfd, buffer);
	return NULL;
}

//enters a new client session
ActiveAccountInfo startaccount(char * accountname, int clientfd)
{
	//allocate memory for new active account
	ActiveAccountInfo ret_acc = (ActiveAccountInfo) malloc(sizeof(ActiveAccountInfo));
	char buffer[256];
	int i = 0;

	//lock the lockbank mutex to prevent other threads from modifying the bank data structure
	//while this thread searches it
	pthread_mutex_lock(&mutex_bank);

	//for each bank entry in the bank
	for(i; i < MAX_ACCOUNTS; i++)
	{
		//if the entry is taken
		if(Bank[i].taken)
		{
			//and the names are equal
			if(!strcmp(Bank[i].account->name, accountname))
			{
				//continually try and lock the mutex corresponding to the account
				//if account is locked by another thread, continually try locking every 3 seconds
				//when the other thread is done with the account this thread will lock it
				while(pthread_mutex_trylock(&mutex_account[i]) != 0)
				{
					strcpy(buffer, "Unable to start account: Account already in session.\nRetrying in 2 seconds...\n");
					WriteToClient(clientfd, buffer);
					throttle(2);
				}
				//this should never execute during successful operation
				//any account that is in session should ALWAYS be locked
				if(Bank[i].account->insession)
					printf("Internal error: should never try lock on an account in session.\n");

				//flag account as insession
				//store active account to be returned
				//unlock bank mutex and send success message to client
				Bank[i].account->insession = 1;
				ret_acc->account = Bank[i].account;
				ret_acc->entryno = i;
				pthread_mutex_unlock(&mutex_bank);
				sprintf(buffer, "Session started for account: %s.\n", accountname);
				WriteToClient(clientfd, buffer);
				return ret_acc;
			}
		}
	}
	//account wasn't found, unlock bank mutex and send message to client
	pthread_mutex_unlock(&mutex_bank);
	strcpy(buffer, "Unable to start account: No account with this name exists.\n");
	WriteToClient(clientfd, buffer);
	return NULL;
}

//credits a specified account a specified amount of money
ActiveAccountInfo creditamount(float amount, ActiveAccountInfo ActiveAccount, int clientfd)
{
	char buffer[256];

	//adds the amount to the active account's balance
	ActiveAccount->account->balance += amount;

	//print success message to client and return ActiveAccount
	sprintf(buffer, "Successfully credited $%'.2f to account: %s.\n", amount, ActiveAccount->account->name);
	WriteToClient(clientfd, buffer);
	return ActiveAccount;
}

//debits a specified amount of money from a specified account
ActiveAccountInfo debitamount(float amount, ActiveAccountInfo ActiveAccount, int clientfd)
{
	char buffer[256];
	float newbalance = 0;

	//subtract debit amount from active account balance and store in new amount
	newbalance = ActiveAccount->account->balance - amount;
	//if the new amount is less than 0 the debit is invalid
	if(newbalance < 0)
	{
		strcpy(buffer, "Unable to debit account: Debit request would leave account balance under $0.00.\n");
		WriteToClient(clientfd, buffer);
		return ActiveAccount;
	}
	//if the newamount is greater than or equal to zero, successfully debit account
	ActiveAccount->account->balance = newbalance;

	//send success message to client and return ActiveAccount
	sprintf(buffer, "Successfully debited $%'.2f from account: %s.\n", amount, ActiveAccount->account->name);
	WriteToClient(clientfd, buffer);
	return ActiveAccount;
}

//sends an informational message containing active account balance to client
ActiveAccountInfo balance(ActiveAccountInfo ActiveAccount, int clientfd)
{
	char buffer[256];
	
	//write balance to client
	sprintf(buffer, "Account current balance: $%'.2f\n", ActiveAccount->account->balance);
	WriteToClient(clientfd, buffer);
	return ActiveAccount;
}

//ends an account session
ActiveAccountInfo finish(ActiveAccountInfo ActiveAccount, int clientfd)
{
	char buffer[256];
	int i = -1;

	//set ActiveAccount insession to 0
	ActiveAccount->account->insession = 0;
	i = ActiveAccount->entryno;

	//unlocks the newly finished account for use by other threads
	pthread_mutex_unlock(&mutex_account[i]);

	//send a success message to client if unless we are finishing as a result of an abrupt disconnect
	if(clientfd)
	{
		sprintf(buffer, "Successfully finished account: \"%s\" is no longer in session.\n", ActiveAccount->account->name);
		WriteToClient(clientfd, buffer);
	}
	free(ActiveAccount);
	return NULL;
}

//validates data to be given to open command
int ValidateOpen(int clientfd, char * accountname, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];

	//if no argument provided to open, send error message to client
	if(accountname == NULL)
	{
		strcpy(buffer, "No argument provided to open. Requires: <account name>.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}
	//if an account is already active a new one cannot be created
	if(ActiveAccount != NULL)
	{
		strcpy(buffer, "Cannnot open new account: Another account is already in session.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}
	return 1;
}

//validates data to be given to start command
int ValidateStart(int clientfd, char * accountname, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];

	//if the account name is null there is no specified account to start
	if(accountname == NULL)
	{
		strcpy(buffer, "No argument provided to start. Requires: <account name>.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//if an account is already active, tell client only one session may be active at a time
	if(ActiveAccount != NULL)
	{
		strcpy(buffer, "Unable to start new session: Another account is already in session. \n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//if none of the above are true attempt to start a new session for the provided accountname
	return 1;
}

//validates data to be sent to credit command
int ValidateCredit(int clientfd, char * amountarg, float * amount, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];
	char * endptr;

	//if there is no account in session it is impossible to credit an account
	if(ActiveAccount == NULL)
	{
		strcpy(buffer, "Unable to credit account: No account is currently in session.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//if no argument provided, print error to client and return NULL
	if(amountarg == NULL)
	{
		strcpy(buffer, "No argument provided to credit. Requires: <amount>\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//attempt to convert retrieved arg into float value
	errno = 0;
	//store amount argument in float value pointed to by amount
	*amount = strtof(amountarg, &endptr);
	//if there was an error during conversion send a message containing this error to the client
	if(errno)
	{
		sprintf(buffer, "Error retrieving <amount> argument: %s\n", strerror(errno));
		WriteToClient(clientfd, buffer);
		return 0;
	}
	//if invalid data was found during conversion send message to client
	if(*endptr != 0)
	{
		sprintf(buffer, "Invalid data found in <amount> argument: %s\n", endptr);
		WriteToClient(clientfd, buffer);
		return 0;
	}
	//if none of the above, the input is valid and we start command
	return 1;
}

//valides the data to be given to the debit command
int ValidateDebit(int clientfd, char * amountarg, float * amount, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];
	char * endptr;

	//if no account is in session it is impossible to debit the account
	if(ActiveAccount == NULL)
	{
		strcpy(buffer, "Unable to debit account: No account is currently in session.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//if no argument provided, print error to client and return NULL
	if(amountarg == NULL)
	{
		strcpy(buffer, "No argument provided to debit. Requires: <amount>\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}

	//attempt to convert retrieved arg into float value
	errno = 0;
	//store the retrieved amount into the float value pointed to be amount
	*amount = strtof(amountarg, &endptr);
	//if there was an error during conversion send a message to the client containing this error
	if(errno)
	{
		sprintf(buffer, "Error retrieving <amount> argument: %s\n", strerror(errno));
		WriteToClient(clientfd, buffer);
		return 0;
	}
	//if invalid data was found during conversion send an error message to the client
	if(*endptr != 0)
	{
		sprintf(buffer, "Invalid data found in <amount> argument: %s\n", endptr);
		WriteToClient(clientfd, buffer);
		return 0;
	}
	return 1;
}

//validates the data to be given to the balance command
int ValidateBalance(int clientfd, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];

	//unable to retrieve balance if there is no account in session
	if(ActiveAccount == NULL)
	{
		strcpy(buffer, "Unable to retrieve balance: No account currently in session.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}
	return 1;
}

//validates the data to be given to the finish command
int ValidateFinish(int clientfd, ActiveAccountInfo ActiveAccount)
{
	char buffer[256];

	//if there is no account in session it is impossible to end the session
	if(ActiveAccount == NULL)
	{
		strcpy(buffer, "Unable to finish account: No account currently in session.\n");
		WriteToClient(clientfd, buffer);
		return 0;
	}
	return 1;
}



#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "server.h"



void * session_acceptor(void * args)
{
	int sockfd = -1;
	int newsockfd = -1;
	int portno = 11909;

	struct sockaddr_in client_addr_info;
	struct sockaddr_in server_addr_info;
	int clientlen = sizeof(client_addr_info);

	void * client_session_args;
	pthread_t client_session_t;
	pthread_attr_t client_session_attributes;

	//create listening server socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		fprintf(stderr, "Error creating socket\n");
		exit(EXIT_FAILURE);
	}

	//initialize bytes in server_addr_info to 0 and set the following properties
	bzero((char *) &server_addr_info, sizeof(server_addr_info));
	server_addr_info.sin_port = htons(portno);
	server_addr_info.sin_family = AF_INET;
	server_addr_info.sin_addr.s_addr = INADDR_ANY;

	//bind the created server socket to newly created server_addr_info struct
	if(bind(sockfd, (struct sockaddr *) &server_addr_info, sizeof(server_addr_info)) < 0)
	{
		perror("Error binding socket");
		exit(EXIT_FAILURE);
	}

	//continually listen and accept new connection requests, placing each client in its own client_session thread
	while(1)
	{
		printf("Listening...\n");
		listen(sockfd, 5);
		newsockfd = accept(sockfd, (struct sockaddr *) &client_addr_info, &clientlen);
		if(newsockfd < 0)
			perror("Error connecting to client");
		else
		{
			client_session_args = calloc(1, sizeof(newsockfd));
			if(client_session_args == NULL)
			{
				printf("Error allocating memory\n");
				exit(EXIT_FAILURE);
			}
			bcopy(&newsockfd, client_session_args, sizeof(newsockfd));
			pthread_attr_init(&client_session_attributes);
			pthread_attr_setdetachstate(&client_session_attributes, PTHREAD_CREATE_DETACHED);
			pthread_attr_setscope(&client_session_attributes, PTHREAD_SCOPE_SYSTEM);
			pthread_create(&client_session_t, &client_session_attributes, client_session, client_session_args);
			pthread_attr_destroy(&client_session_attributes);
		}
	}
}

void * client_session(void * args)
{
	int clientfd = *((int *) args);
	char * buffer = NULL;

	write_to_socket(clientfd, "Connection established.");

	while(1)
	{
		write_to_socket(clientfd, "Enter your command.");
		buffer = read_from_socket(clientfd);
		write_to_socket(clientfd, "Command received.");
	}


}

int execute_command_active(int sockfd, char * command)
{
	printf("Command received: %s", command);
	return 1;
}

int execute_command_inactive(int sockfd, char * command)
{
 	char * token = NULL;
 	char * saveptr = NULL;

	printf("Command received: %s", command);
	token = strtok_r(command, " \n", &saveptr);

	//if the command was open
	if(!strcmp(token, "open"))
	{
		//next token should be accountname
		token = strtok_r(NULL, " \n", &saveptr);
		if(token == NULL)
		{
			write_to_socket(sockfd, "Unable to open account. No account name provided.\n");
			return 1;
		}
		return open_account(sockfd, token);
	}


 	return 1;
}

int open_account(int sockfd, char * accname)
{
	//create mutex so no other threads can modify AccountList while it is in use here
	pthread_mutex_t AccountList_mutex;
	//allocate mem for new AccountInfo struct
	AccountInfo newaccount = (AccountInfo) malloc(sizeof(struct AccountInfo_));
	int i = 0;
	//if the name provided is over 100 characters the account name is invalid
	if(strlen(accname) > 100)
	{
		write_to_socket(sockfd, "Account name too long. Unable to create.\n");
		free(newaccount);
		return 0;
	}

	if(account_name_taken(accname))
	{
		write_to_socket(sockfd, "Account name taken. Unable to create.");
		free(newaccount);
		return 0;
	}

	//initialize new AccountInfo struct
	strcpy(newaccount->name, accname);
	newaccount->balance = 0;
	newaccount->insession = 0;
	pthread_mutex_lock(&AccountList_mutex);
	for(i; i < MAX_ACCOUNT_ARRAY_SIZE; i++)
	{
		if(!AccountList[i].entrytaken)
		{
			AccountList[i].entrytaken = 1;
			AccountList[i].account = newaccount;
			write_to_socket(sockfd, "Account successfully created.\n");
			pthread_mutex_unlock(&AccountList_mutex);
			return 1;
		}
	}
	pthread_mutex_unlock(&AccountList_mutex);
	write_to_socket(sockfd, "Account limit reached: 20");
	return 0;
}

int account_name_taken(char * name)
{
	int i = 0;
	for(i; i < MAX_ACCOUNT_ARRAY_SIZE; i++)
	{
		if(!strcmp(AccountList[i].account->name, name))
			return 1;
	}
	return 0;
}

char * read_from_socket(int sockfd)
{
	char * buffer = (char *) calloc(256, 1);
	printf("attempting to read\n");
	printf("buffercontents %s buffernlen: %zu\n", buffer, strlen(buffer));
	if(read(sockfd, buffer, 255) < 0)
	{
		printf("Error reading from socket.\n");
		exit(EXIT_FAILURE);//sort this out
	}
	return buffer;
}

void write_to_socket(int sockfd, char * message)
{
	char buffer[256];
	bzero(buffer, 256);
	strcpy(buffer, message);
	if(write(sockfd, buffer, strlen(buffer) + 1) < 0)
	{
		printf("Error writing to socket\n");
		exit(EXIT_FAILURE);//exit stuff
	}
}


int main(int argc, char ** argv)
{
	//declarations for the session_acceptor pthread and its attributes and arguments
	pthread_t session_acceptor_t;
	pthread_attr_t session_acceptor_attributes;

	//allocate memory for an AccountList of size 20 on the heap
	AccountList = (AccountEntry) calloc(MAX_ACCOUNT_ARRAY_SIZE, sizeof(struct AccountEntry_));
	if(AccountList == NULL)
	{
		fprintf(stderr, "Error allocating memory for accounts.\n");
		exit(EXIT_FAILURE);
	}
	bzero(AccountList, sizeof(struct AccountEntry_) * MAX_ACCOUNT_ARRAY_SIZE);

	//create attributes for the session_acceptor pthread
	pthread_attr_init(&session_acceptor_attributes);
	pthread_attr_setdetachstate(&session_acceptor_attributes, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&session_acceptor_attributes, PTHREAD_SCOPE_SYSTEM);
	//create the pthread for the session_acceptor and destroy its attributes
	pthread_create(&session_acceptor_t, &session_acceptor_attributes, session_acceptor, NULL);
	pthread_attr_destroy(&session_acceptor_attributes);


	//fix NULL arg
	pthread_join(session_acceptor_t, NULL);
	exit(EXIT_SUCCESS);
}

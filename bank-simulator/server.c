/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#include "sessioncommands.h"
#include "server.h"

//display errors and exit
void error(char * msg)
{
	perror(msg);
	exit(1);
}

//session acceptor thread will continually search for new client connection requests and place them in
//their own client session threads
void * SessionAcceptor(void * args)
{
	//get sockfd and declare vars
	int sockfd = *((int *) args);
	int clilen = -1;
	struct sockaddr_in client_address_info;
	int i = 0;

	//create sockopt args for all client sockets
	//used to timeout write commands
	write_timeout = (struct timeval *) malloc(sizeof(struct timeval));
	write_timeout->tv_sec = 0;
	write_timeout->tv_usec = 50000;
	return_len = (socklen_t *) malloc(sizeof(socklen_t));
	*return_len = (socklen_t) sizeof(struct timeval);


	//determine the size of a client_address_info struct
	clilen = sizeof(client_address_info);

	//initialize list of ClientSessionInfo_ structs
	//this list maintains the pthread handle and socket file descriptor for each client
	//that was created
	ClientSessionInfoList = (ClientSessionInfo) malloc(sizeof(struct ClientSessionInfo_));

	//initialize pthread_mutex_t variables for each new client session thread to share
	//bank mutex protects Bank structure
	//account mutexes protect individual accounts
	pthread_mutex_init(&mutex_bank, NULL);
	for(i; i < MAX_ACCOUNTS; i++)
		pthread_mutex_init(&mutex_account[i], NULL);


	//continually search for new clients
	SearchNewClients(sockfd, client_address_info, clilen);

	//display exit message and exit thread
	pthread_exit(NULL);
}

//continually search for new clients
void SearchNewClients(int sockfd, struct sockaddr_in client_address_info, int clilen)
{
	//declare vars
	int newsockfd = -1;

	//create poll struct to poll for clients looking to connect
	struct pollfd accept_poll;
	accept_poll.fd = sockfd;
	accept_poll.events = POLLIN;
	int i = -1;

	//continually search while server is online
	while(server_online)
	{
		//poll for incoming connetion requests
		i = poll(&accept_poll, 1, 50);
		if(i)
		{
			if(i < 0)
				error("Error polling accept socket");

			//accept connection request or display error
			bzero(&client_address_info, clilen);
			newsockfd = accept(sockfd, (struct sockaddr *) &client_address_info, &clilen);
			if (newsockfd < 0)
				error("ERROR on accept");

			//send the newly connected client into a client session thread
			printf("Accepted new client!\n");
			CreateClientSession(newsockfd);
		}
	}
}

//creates a client session thread for a client
void CreateClientSession(int clientfd)
{
	//allocate heap mem for thread args (socket file descriptor)
	int * ClientSession_args = (int *) malloc(sizeof(int));
	*ClientSession_args = clientfd;

	//set socket options to timeout write commands
	if(setsockopt(clientfd, SOL_SOCKET, SO_SNDTIMEO, write_timeout, *return_len) < 0)
		error("Error setting socket options");


	//create new session thread handle and malloc heap mem to store it
	pthread_t newsession;
	pthread_t * newsessionptr = (pthread_t *) malloc(sizeof(pthread_t));
	*newsessionptr = newsession;

	//create a new struct to store info on client session
	struct ClientSessionInfo_ newcsi;
	newcsi.thandle = newsessionptr;
	newcsi.clientfd = clientfd;

	//add the client session info struct to the array of all client session info structs
	//reallocates size to allow for one more client session info struct
	ClientSessionInfoList[numclients] = newcsi;
	ClientSessionInfoList = (ClientSessionInfo) realloc(ClientSessionInfoList, sizeof(struct ClientSessionInfo_) * (numclients + 2));

	//create thread attributes and create thread
	pthread_attr_t ClientSession_attr;
	pthread_attr_init(&ClientSession_attr);
	pthread_attr_setdetachstate(&ClientSession_attr, PTHREAD_CREATE_JOINABLE);	
	pthread_attr_setscope(&ClientSession_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(ClientSessionInfoList[numclients++].thandle, &ClientSession_attr, ClientSession, (void *) ClientSession_args);

	//destroy attributes
	pthread_attr_destroy(&ClientSession_attr);		
}

//sleep thread for the specified amount of time
unsigned int throttle(unsigned int sec)
{
	if(sleep(sec) != 0)
		throttle(sec);
}

//each client is placed in its own client session thread
//this thread handles bank operations for each client and uses mutexes to protect shared data
void * ClientSession(void * args)
{
	//get clientfd and declare vars
	int clientfd = *((int *) args);
	ActiveAccountInfo ActiveAccount = NULL;
	char buffer[256];
	int i = -1;

	//create poll struct to poll for incoming messages
	struct pollfd read_poll;
	read_poll.fd = clientfd;
	read_poll.events = POLLIN;

	//print new client connected message and send welcome message to client
	printf("New client connected.\n");
	strcpy(buffer, "\n/=======Welcome to Bank System=======\\\n\n\t/==Session Commands==\\\n\t open <account name>\n\t start <account name>\n");
	strcat(buffer, "\t credit <amount>\n\t debit <amount>\n\t balance\n\t finish\n\t exit\n\t\\====================/\n\n");
	WriteToClient(clientfd, buffer);

	//continually read and handle new commands while the server is online
	while(server_online)
	{
		//poll for incoming messages from client
		i = poll(&read_poll, 1, 100);
		if(i)
		{
			//attempt to read from client
			//if the read failed display message, set its active account to not in session, free allocated mem, and exit thread
			if(!ReadFromClient(clientfd, buffer, i))
			{
				//print notification of abrupt client disconnect
				printf("Client disconnected without exit command.\n");

				//if the client had an ative account finish it
				if(ActiveAccount != NULL)
					finish(ActiveAccount, 0);
				
				//free thread args and exit thread
				free(args);
				pthread_exit(NULL);
			}
			//remove newline character from the end of buffer and execute the client's command
			buffer[strlen(buffer) - 1] = 0;
			if(strlen(buffer) > 0)
				ActiveAccount = ExecuteCommand(buffer, ActiveAccount, clientfd);
		}
	}
	//when the server goes offline, send notice to the client, free thread args and exit thread
	strcpy(buffer, "Server has shutdown.\n");
	WriteToClient(clientfd, buffer);
	free(args);
	pthread_exit(NULL);
}

//reads a message from the client
int ReadFromClient(int clientfd, char * buffer, int poll_val)
{
	int n = -1;

	//display error if poll returned < 0
	if(poll_val < 0)
		error("Error polling client socket");

	//zero out the buffer and read in client's message
	bzero(buffer, 256);
	n = read(clientfd, buffer, 255);	
	if(n < 0)
		error("Error reading from client");

	//if read returned 0 then the client disconnected: return 0
	if(!n)
		return 0;

	//return 1 for successful read
	return 1;


}

//executes the command received from the client
ActiveAccountInfo ExecuteCommand(char * commandstr, ActiveAccountInfo ActiveAccount, int clientfd)
{
	//declare vars
	char * command;
	char * saveptr;
	char * cmdarg;
	char * endptr;
	float amount;
	char buffer[256];
	int i = -1;

	//tokenize the command up to the first space character
	//uses saveptr as reentrant to ensure tokenization is threadsafe
	command = strtok_r(commandstr, " ", &saveptr);

	//if the commmand was "open" get the arg and validate the data
	//if data is valid execute open command
	if(!strcmp(command, "open"))
	{
		cmdarg = strtok_r(NULL, " ", &saveptr);
		if(ValidateOpen(clientfd, cmdarg, ActiveAccount))
			return openaccount(cmdarg, clientfd);
		else
			return ActiveAccount;
	}

	//if the commmand was "start" get the arg and validate the data
	//if data is valid execute startcommand
	if(!strcmp(command, "start"))
	{
		cmdarg = strtok_r(NULL, " ", &saveptr);
		if(ValidateStart(clientfd, cmdarg, ActiveAccount))
			return startaccount(cmdarg, clientfd);
		else
			return ActiveAccount;
	}

	//if the commmand was "credit" get the arg and validate the data
	//if data is valid execute credit command
	if(!strcmp(command, "credit"))
	{
		cmdarg = strtok_r(NULL, " ", &saveptr);	
		if(ValidateCredit(clientfd, cmdarg, &amount, ActiveAccount))
			return creditamount(amount, ActiveAccount, clientfd);
		else
			return ActiveAccount;
	}

	//if the commmand was "debit" get the arg and validate the data
	//if data is valid execute debit command
	if(!strcmp(command, "debit"))
	{
		cmdarg = strtok_r(NULL, " ", &saveptr);
		if(ValidateDebit(clientfd, cmdarg, &amount, ActiveAccount))
			return debitamount(amount, ActiveAccount, clientfd);
		else
			return ActiveAccount;
	}

	//if the commmand was "balance" validate the data
	//if data is valid execute balance command
	if(!strcmp(command, "balance"))
	{
		if(ValidateBalance(clientfd, ActiveAccount))
			return balance(ActiveAccount, clientfd);
		else
			return ActiveAccount;
	}

	//if the commmand was "finish" validate the data
	//if data is valid execute finish command
	if(!strcmp(command, "finish"))
	{
		if(ValidateFinish(clientfd, ActiveAccount))
			return finish(ActiveAccount, clientfd);
		else
			return ActiveAccount;
	}

	//if the command was exit, display informational message and send disconnect message to client
	if(!strcmp(command, "exit"))
	{
		strcpy(buffer, "Disconnected from server.\n");
		WriteToClient(clientfd, buffer);
		printf("Client has disconnected.\n");
		
		//if the client had an active account finish it
		if(ActiveAccount != NULL)
			finish(ActiveAccount, 0);

		pthread_exit(NULL);
	}
	//invalid command: send error message to client and try again
	sprintf(buffer, "Invalid command: %s\nTry again.\n", command);
	WriteToClient(clientfd, buffer);
	return ActiveAccount;
}

//joins all client threads ensuring each client session has properly ended
void JoinClientThreads()
{
	//declare vars
	int i = 0;
	int sockfd = -1;
	pthread_t * thandle;

	//execute only if the numclients that have connected is greater than zero
	if(numclients > 0)
	{
		//for each client
		for(i; i < numclients; i++)
		{
			//obtain its respective sockfd and thread handler
			sockfd = ClientSessionInfoList[i].clientfd;
			thandle = ClientSessionInfoList[i].thandle;

			//join the thread handler and free it from the heap
			pthread_join(*thandle, NULL);
			free(ClientSessionInfoList[i].thandle);
		}
		//free the mem allocated for the client info array
		free(ClientSessionInfoList);
	}
}

//print out list of all accounts or display that the bank is empty
void PrintAccountList()
{
	int i = 0;
	int j = 0;

	printf("/=================BANK SERVER STATE=================\\\n");

	//for each account bank
	for(i; i < MAX_ACCOUNTS; i++)
	{
		//if the account exists
		if(Bank[i].taken)
		{
			j = 1;
			//print out message containing information on the account
			printf("|---~---~---~---~---~---~---~---~---~---~---~---\n");
			printf("| Account Name:\t\t%s\n", Bank[i].account->name);
			printf("| Account Balance:\t$%'.2f\n", Bank[i].account->balance);
			if(Bank[i].account->insession)
				printf("| IN SESSION\n");
			printf("|---~---~---~---~---~---~---~---~---~---~---~---\n");
		}
	}
	//display closing message or that bank has no accounts
	if(j)
		printf("\\===================================================/\n");
	else
		printf("---~---~---Bank has no accounts---~---~---\n");
}

//writes a message to specified client
void WriteToClient(int clientfd, char * message)
{
	//declare vars
	int n = 0;
	char buffer[256];

	//if the length of the message is not greater than 0 display warning and return
	if(!strlen(message) > 0)
	{
		printf("Warning: trying to print empty message\n");
		return;
	}

	//copy message into buffer
	strcpy(buffer, message);
	while(n < 1)
	{
		//set errno to 0 and attempt to write to client
		//timeout after 50ms as specified in timeval struct
		errno = 0;
		n = write(clientfd, buffer, strlen(buffer) + 1);
		//if there is no error, write was successful and return
		if(errno == 0) return;
		//if it was any of the specified errors, write timed out and try again
		//if the error was not specified, display it and exit
		if(!(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS))
			error("Error writing to socket");
	}
}

//creates a server socket for use by session acceptor thread
int CreateServerSocket(int portno)
{
	//declare vars
	int sockfd = -1;
	struct sockaddr_in server_address_info;
	
	//create socket and display error on failure
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	//set sock options to reuse the addr
	//this is useful for testing purposes: able to immediately reuse socket after process termination
	int optflag;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optflag, sizeof(int)) < 0)
		error("Error setting server socket options");

	//zero out server_address_info struct and set appropriate fields
	bzero((char *) &server_address_info, sizeof(server_address_info));
	server_address_info.sin_port = htons(portno);	 
	server_address_info.sin_family = AF_INET;
	server_address_info.sin_addr.s_addr = INADDR_ANY;

	//bind socket and display error on failure     
	if (bind(sockfd, (struct sockaddr *) &server_address_info, sizeof(server_address_info)) < 0)
		error("ERROR on binding");

	//make the socket a listneing socket and return sockfd
	listen(sockfd, 5);
	return sockfd;
}

//signal handler to handle interrupt signal
void ServerShutdown(int signum)
{
	//if the server was already set to offline (from a previous interrupt signal) ignore signal as server
	//shutdown is already in progress
	if(!server_online) return;

	//set server online status to 0 (not online) and join all client threads
	server_online = 0;
	JoinClientThreads();
}

//create a detached thread responsible for displaying server state every 20 seconds
//thread uses an alarm
void CreateDisplayAlarm_T()
{
	//set attributes for and create DisplayAlarm_t
	pthread_attr_t DisplayAlarm_attr;
	pthread_attr_init(&DisplayAlarm_attr);
	pthread_attr_setdetachstate(&DisplayAlarm_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&DisplayAlarm_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&DisplayAlarm_t, &DisplayAlarm_attr, DisplayAlarm, NULL);
}

//create thread responsible for accepting new client connection requests
void CreateSessionAcceptor_T(void * SessionAcceptor_args)
{
	pthread_attr_t SessionAcceptor_attr;
	pthread_attr_init(&SessionAcceptor_attr);
	pthread_attr_setdetachstate(&SessionAcceptor_attr, PTHREAD_CREATE_JOINABLE);	
	pthread_attr_setscope(&SessionAcceptor_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_create(&SessionAcceptor_t, &SessionAcceptor_attr, SessionAcceptor, (void *) SessionAcceptor_args);
	pthread_attr_destroy(&SessionAcceptor_attr);	
}

//thread calls alarm to send a SIGALRM signal to the process every 20 seconds
//thread is detached: since we are not waiting on data from it it does not need to be joined
void * DisplayAlarm(void * args)
{
	//create semaphore to block next alarm call
	if(sem_init(&sem_alarm, 0, 0) < 0)
		error("Unable to create semaphore");

	//continue while server is online
	while(server_online)
	{
		//set alarm to go off in ALARM_TIMER seconds
		alarm(ALARM_TIMER);
	
		//block until the semaphore is posted in the alarm signal handler	
		if(sem_wait(&sem_alarm) < 0)
			error("Error waiting for semaphore");	
	}
}

//signal handler to handle SIGALRM sent to process as a result of the alarm created in the display alarm thread
void DisplayServerState(int signum)
{
	//print out the list and post the alarm semaphore so display alarm thread will reset the alarm
	PrintAccountList();
	if(sem_post(&sem_alarm) < 0)
		error("Error posting to semaphore");
}

int main(int argc, char ** argv)
{
	//create signals
	signal(SIGINT, ServerShutdown);
	signal(SIGALRM, DisplayServerState);

	//get portno and sockfd
	int portno = 9709;
	int sockfd = CreateServerSocket(portno);

	int i = 0;

	//allocate mem for the bank and all of its accounts
	Bank = (BankEntry) calloc(MAX_ACCOUNTS, sizeof(struct BankEntry_));
	for(i; i < MAX_ACCOUNTS; i++)
		Bank[i].account = (AccountInfo) calloc(1, sizeof(struct AccountInfo_));

	//allocate mem for session acceptor args	
	int * SessionAcceptor_args = (int *) malloc(sizeof(int));
	*SessionAcceptor_args = sockfd;

	//create the session acceptor thread and the display alarm thread
	CreateSessionAcceptor_T((void *) SessionAcceptor_args);
	CreateDisplayAlarm_T();

	//block until session acceptor thread exits
	//this will only happen after a shutdown signal (SIGINT) has been sent to the process
	pthread_join(SessionAcceptor_t, NULL);

	//free all allocated mem
	for(i; i < MAX_ACCOUNTS; i++)
		free(Bank[i].account);
	free(Bank);
	free(SessionAcceptor_args);
	free(write_timeout);
	free(return_len);

	//print that server has successfully completed its shutdown procedures and exit process
	printf("\nServer successfully shut down.\n");	
	exit(EXIT_SUCCESS);
}
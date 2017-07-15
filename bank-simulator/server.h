/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#ifndef SERVER_H
#define SERVER_H	

#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <poll.h>

#define MAX_ACCOUNT_NAME_LEN 101
#define MAX_ACCOUNTS 20
#define ALARM_TIMER 20

//struct to hold account info
typedef struct AccountInfo_ * AccountInfo;
struct AccountInfo_
{
	char name[MAX_ACCOUNT_NAME_LEN];
	float balance;
	int insession;
};

//struct to hold a bank entry
typedef struct BankEntry_ * BankEntry;
struct BankEntry_
{
	AccountInfo account;
	int taken;
};

//bank structure utilized by server process and its threads
BankEntry Bank;

//client session info structs store information on each client connected
typedef struct ClientSessionInfo_ * ClientSessionInfo;
struct ClientSessionInfo_
{
	pthread_t * thandle;
	int clientfd;
};

//array to store info on every client accepted and its length
static int numclients = 0;
static ClientSessionInfo ClientSessionInfoList;

//information struct for an active account
typedef struct ActiveAccountInfo_ * ActiveAccountInfo;
struct ActiveAccountInfo_
{
	AccountInfo account;
	int entryno;
};

//struct used as arg when setting sock opt to timeout write commands every 50ms
static struct timeval * write_timeout;
socklen_t * return_len;

//flag to store server online status
static char server_online = 1;

//thread handles for session acceptor and display alarm threads
static pthread_t SessionAcceptor_t;
static pthread_t DisplayAlarm_t;

//mutex variables
static sem_t sem_alarm;
static pthread_mutex_t mutex_bank;
static pthread_mutex_t mutex_account[MAX_ACCOUNTS];

//displays error and exits process
void error(char * msg);

//session acceptor thread function and its helper functions
void * SessionAcceptor(void * args);
void SearchNewClients(int sockfd, struct sockaddr_in server_address_info, int clilen);
void CreateSessionAcceptor_T(void * SessionAcceptor_args);

//creates a server socket given a port number
int CreateServerSocket(int portno);

//client session thread and its helper functions
void * ClientSession(void * args);
void CreateClientSession(int clientfd);
int ReadFromClient(int clientfd, char * buffer, int poll_val);

//joins all client threads
void JoinClientThreads();

//prints list of all accounts in bank
void PrintAccountList();

//writes a message to a specified client
void WriteToClient(int clientfd, char * message);

//executes a command received from a client
ActiveAccountInfo ExecuteCommand(char * commandstr, ActiveAccountInfo ActiveAccount, int clientfd);

//display alarm thread to send SIGALRM signal to process every 20 seconds
void CreateDisplayAlarm_T();
void * DisplayAlarm(void * args);

//displays server state when SIGARLM is sent to process
void DisplayServerState(int signum);

//shuts down server when SIGINT is sent to process
void ServerShutdown(int signum);


#endif
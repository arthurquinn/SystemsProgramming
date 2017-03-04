/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <poll.h>

//client connection status flag
static char connected;

//thread handles
pthread_t ReadFromServer_t;
pthread_t WriteToServer_t;

//displays error messages
void error(char * msg);

//read and write thread functions
void * ReadFromServer(void * args);
void * WriteToServer(void * args);

//waits for specified amount of seconds
unsigned int throttle(unsigned int sec);

//creates a client socket and tries to connect once every 3 seconds
int CreateClientSocket(char * ipaddress, int portno);
int AttemptConnect(int sockfd, struct sockaddr_in server_address_info);










#endif
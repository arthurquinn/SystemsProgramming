/* Arthur Quintanilla and Razeno Khan
 * Project 5 - Bank System
 */

#include "client.h"

//display error message and exit
void error(char * msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

//sleep the process for the specified amount of seconds
unsigned int throttle(unsigned int sec)
{
	if(sleep(sec) != 0)
		throttle(sec);
}

//read a message from the server
void * ReadFromServer(void * args)
{
	//declare vars
	int sockfd = *((int *) args);
	int n = -1;
	char buffer[256];

	//create a poll struct to poll for server messages
	struct pollfd serv_poll;
	serv_poll.fd = sockfd;
	serv_poll.events = POLLIN;
	int i = -1;

	//continually search for messages, calls pthread_exit to terminate	
	while(1)
	{
		//poll server for messages
		i = poll(&serv_poll, 1, 50);
		if(i)
		{
			if(i < 0)
				error("Error polling server");
			//zero out buffer and read message
			bzero(buffer, 256);
			n = read(sockfd, buffer, 255);
			if(n < 0)
				error("Error reading from socket");
			//display message and flush stdout
			printf("Server Message: %s", buffer);
			fflush(stdout);	
			//if the server sent a disconnect message, set connected to 0, close socket, and exit thread
			if(!strcmp(buffer, "Disconnected from server.\n") || !strcmp(buffer, "Server has shutdown.\n"))
			{
				connected = 0;
				close(sockfd);
				pthread_exit(NULL);
			}
		}
	}
}

//write message to server
void * WriteToServer(void * args)
{
	//declare vars
	int sockfd = *((int *) args);
	char buffer[256];
	int n = -1;

	//create poll struct to poll stdin for input
	struct pollfd stdin_poll;
	stdin_poll.fd = STDIN_FILENO;
	stdin_poll.events = POLLIN;
	int i = -1;

	//continually search for stdin input while client is connected to server
	while(connected)
	{
		//poll stdin
		i = poll(&stdin_poll, 1, 50);
		if(i)
		{
			if(i < 0)
				error("Error polling stdin");
			//zero out buffer and get input
			bzero(buffer, 256);
			fgets(buffer, 255, stdin);
			throttle(2);
			//after 2 seconds, write message to server
			n = write(sockfd, buffer, strlen(buffer) + 1);
			if(n < 0)
				error("Error writing to socket");
			//if the message sent was "exit\n" then exit the thread
			if(!strcmp(buffer, "exit\n"))
				pthread_exit(NULL);
		}
	}
}

int CreateClientSocket(char * ipaddress, int portno)
{
	//declare vars
	int sockfd = -1;
	int i = 0;
	struct hostent * server_ip_address;
	struct sockaddr_in server_address_info;

	//convert the string ipaddress into struct hostent * server_ip_address and display error if no such host exists...then exit
    server_ip_address = gethostbyname(ipaddress);
    if(server_ip_address == NULL)
	{
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
				
	//build the socket, if an error is detected then display it and exit
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) 
		error("ERROR creating socket");

	//zero out the server_address_info struct, set its properties, and copy it into hostent struct
    bzero((char *) &server_address_info, sizeof(server_address_info));
    server_address_info.sin_family = AF_INET;
	server_address_info.sin_port = htons(portno);
    bcopy((char *)server_ip_address->h_addr, (char *)&server_address_info.sin_addr.s_addr, server_ip_address->h_length);

	//Continually attempt to connect every 3 seconds until a connection is established
	sockfd = AttemptConnect(sockfd, server_address_info);
	return sockfd;
}

int AttemptConnect(int sockfd, struct sockaddr_in server_address_info)
{
	int n = -1;
	//while a connection has not yet been found
	while(n)
	{
		//initialize errno and try to connect
		errno = 0;
		n = connect(sockfd, (struct sockaddr *) &server_address_info, sizeof(server_address_info));
		//if errno is zero connection was successful
		if(errno == 0)
			printf("Successfully connected to server.\n");
		//if connection was refused try again in 3 seconds
		else if(errno == ECONNREFUSED)
		{
			printf("Server not found: Reattempting in 3 seconds...\n");
			throttle(3);
		}
		//else display an error other than ECONNREFUSED and exit
		else
			error("Error connecting socket");
	}
	//set connected status to 1 and return socket
	connected = 1;
	return sockfd;
}

int main(int argc, char ** argv)
{
	// Declare initial vars
    int sockfd = -1;
	int portno = 9709;
  	
  	//initialize connected to 0  	
	connected = 0;

	//display error and exit if invalid number of args
    if (argc != 2)
	{
       fprintf(stderr,"Requires 1 argument: IP Address");
       exit(0);
    }

    //create the socket
    sockfd = CreateClientSocket(argv[1], portno);

    //allocate mem for read and write thread args and set them
	int * ReadFromServer_args = (int *) malloc(sizeof(int));
	int * WriteToServer_args = (int *) malloc(sizeof(int));
	*ReadFromServer_args = sockfd;
	*WriteToServer_args = sockfd;


	//build blank pthread attribute structs and initialize them
	pthread_attr_t ReadFromServer_attr;
	pthread_attr_t WriteToServer_attr;
	pthread_attr_init(&ReadFromServer_attr);
	pthread_attr_init(&WriteToServer_attr);

	//set the initialized attribute struct so that the pthreads created will be joinable
	pthread_attr_setdetachstate(&ReadFromServer_attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_setdetachstate(&WriteToServer_attr, PTHREAD_CREATE_JOINABLE);
	
	//set the initialized attribute struct so that the pthreads created will be kernel threads
	pthread_attr_setscope(&ReadFromServer_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setscope(&WriteToServer_attr, PTHREAD_SCOPE_SYSTEM);

	//build the pthreads
	pthread_create(&ReadFromServer_t, &ReadFromServer_attr, ReadFromServer, (void *) ReadFromServer_args);
	pthread_create(&WriteToServer_t, &WriteToServer_attr, WriteToServer, (void *) WriteToServer_args);

	//destroy attribute structs
	pthread_attr_destroy(&ReadFromServer_attr);
	pthread_attr_destroy(&WriteToServer_attr);
	
	//block until both threads join
	pthread_join(WriteToServer_t, NULL);
	pthread_join(ReadFromServer_t, NULL);

	//free mem allocated for args
	free(ReadFromServer_args);
	free(WriteToServer_args);

	//display message and exit
	printf("Client shutting down.\n");
	exit(EXIT_SUCCESS);
}

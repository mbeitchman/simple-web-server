// Marc Beitchman
// threaded server main
// project 2 - csep551

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

// thread function forward definition
void* handle_request_function(void* data);

// globals
int sockfd;
addrinfo* servinfo;

void ctrlcHandler(int sig)
{
	// clean-up
	freeaddrinfo(servinfo);
	close(sockfd);
	printf("\nClosing threaded server. Goodbye!\n");
	exit(0);
}

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: t-server <port #>\n");
		return 0;
	}
	
	int status = 0;
	sockfd = 0;
	pthread_t tid = {0};
	int i = 0;

	// set-up signal handlers
	signal(SIGINT, ctrlcHandler);
	signal(SIGPIPE, SIG_IGN);
	
	// set-up initial listening socket	
	addrinfo hints = {0};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	status = getaddrinfo(NULL, argv[1], &hints, &servinfo);
	if(status != 0)
	{
		printf("\n getaddrinfo error: %s\n", gai_strerror(status));
		raise(SIGINT);
	}

	sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);	
	if(sockfd == -1)
	{
		printf("\n Unable to open initial listening socket: %s\n", strerror(errno));
		raise(SIGINT);
	}

	status = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
	if(status == -1)
	{
		printf("\n Unable to bind initial listening socket: %s\n", strerror(errno));
		raise(SIGINT);
	}

	status = listen(sockfd, 1024);
	if(status == -1)
	{
		printf("\n Unable to listen on the initialized socket: %s\n", strerror(errno));
		raise(SIGINT);
	}

	printf("\nThreaded Server initialized and listening on socket id %d\n", sockfd);

	// data for accept
	sockaddr_storage their_addr;
	socklen_t addr_size = sizeof their_addr;

	// main loop where we accept new connections and create a thread to handle the 
	// request and send the response
	while(true)	
	{
		// allocate heap memory for the new socket, this is free'd in
		// the thread function
		int* new_fd = (int*)malloc(sizeof(int));
		if(new_fd == NULL)
		{
			printf("\n Unable to allocate memory for socket id!\n");
			raise(SIGINT);
		}

		*new_fd = accept(sockfd, (sockaddr*)&their_addr, &addr_size);
		if(*new_fd == -1)
		{
			printf("\n Unable to accept new connection: %s\n", strerror(errno));
			raise(SIGINT);
		}
		
		status = pthread_create(&tid, NULL, handle_request_function, (void*)new_fd);		
		if(status != 0)
		{
			printf("Error creating the thread!\n");
			raise(SIGINT);
		}

		printf("\n%d threads created\n", ++i);

		status = pthread_detach(tid);
		if(status != 0)
		{
			printf("Error detatching the thread!\n");
			raise(SIGINT);
		} 
	}

	// should never get here, but just in case
	freeaddrinfo(servinfo);
	close(sockfd);
	
	return 0;
}

// thread function
void* handle_request_function(void* data)
{
	int* fd = (int*)data;
	char buffer[4096];
	char* response = NULL;	
	ResourceType requestedResource = UNKNOWN;
	long rlen = 0;
	HTTPRequestParser httpRequestParser;
	HTTPResponseHelper httpResponseHelper;
	HTTPRequestBuffer httpRequestBuffer;
	bool isValidRequest = false;

	do
	{
		int readb = recv(*fd, buffer, 4096, 0);
		if(readb <= 0)
		{
			break;
		}	

		// add the data to our request buffer helper
		httpRequestBuffer.AddData(buffer, readb, &isValidRequest);

		// if the request is not valid, try to read data onece more
		if(!isValidRequest)
		{
			readb = recv(*fd, buffer, 4096, 0);
			if(read > 0)
			{
				readb = httpRequestBuffer.AddData(buffer, readb, &isValidRequest);
			}
		}

		// parse the request and prepare the response
		if(!isValidRequest)
		{
			requestedResource = UNKNOWN;
		}
		else
		{
			requestedResource = httpRequestParser.ParseRequest(buffer);	
		}

		printf("Request received \n");

		switch(requestedResource)
		{
			case A2:

				response = httpResponseHelper.GetA2Response(&rlen);
			
				break;

			case DRACULA:

				response = httpResponseHelper.GetDraculaResponse(&rlen);

				break;

			case MOUNTAIN:

				response = httpResponseHelper.GetMountainResponse(&rlen);

				break;

			case UNKNOWN:
	
				response = httpResponseHelper.GetErrorResponse(&rlen);
		}
		
		int writeb = send(*fd, response, rlen, 0);
		if(writeb != rlen)
		{
			printf("\nERROR: Full response not sent.\n");
			break;
		}

		printf("Response sent \n");
	
	}while(false);

	close(*fd);
	free(fd);

	return 0;
}

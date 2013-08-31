// Marc Beitchman
// event driven server main
// project 2 - csep551

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include "socketlist.h"

// forward defines
void HandleSockets();
void SetNonBlocking(int socket); 
void BuildSocketList();

// globals
fd_set r_fds;
fd_set w_fds;
int highfd = 0;
int sockfd = 0;
int numSocks = 0;
SocketNodeList* socketList;
HTTPResponseHelper* httpResponseHelper;
HTTPRequestParser* httpRequestParser;
addrinfo* servinfo;


void ctrlcHandler(int sig)
{
	// clean up
	delete httpResponseHelper;
	delete httpRequestParser;
	delete socketList;
	close(sockfd);
	freeaddrinfo(servinfo);
	printf("\nClosing server. Goodbye!\n");
	exit(0);
}

int main(int argc, char** argv)
{
	int localNumSocks = 0;
	if(argc != 2)
	{
		printf("Usage: ed-server <port #>\n");
		return 0;
	}	
	
	// set-up signal handlers
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, ctrlcHandler);

	// initialize helpers and socket list
	httpResponseHelper = new HTTPResponseHelper();
	httpRequestParser = new HTTPRequestParser();
	socketList = new SocketNodeList();
	
	// set-up initial listening socket in non-blocking mode
	int status = 0;
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

	SetNonBlocking(sockfd);
	
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

	// add the socket to the socket list
	socketList->AddSocket(sockfd, OPEN);
	numSocks++;

	printf("\nEvent driven Server initialized and listening on socket id %d\n\n", sockfd);

	// time interval for select
	timeval tv = {0};
	tv.tv_sec = 0.5;
	
	// event loop	
	while(true)
	{
		// only print out number of sockets when they change
		if(localNumSocks != numSocks)
		{
			printf("\n%d Sockets Created\n\n", numSocks);
			localNumSocks = numSocks;
		}

		// build list of sockets to check AND pass the r and w lists to select
		BuildSocketList();		

		// check for ready sockets
		status = select(highfd+1, &r_fds, &w_fds, (fd_set*)0, &tv);
		if(status == -1)
		{
			printf("\n Select returned with the following error: %s\n", strerror(errno));
			raise(SIGINT);
		}
		else if(status == 0)
		{
			//printf("\nWaiting on sockets...\n");
		}
		else
		{
			HandleSockets();				
		}
	}

	// should never get here, but just in case
	delete httpResponseHelper;
	delete httpRequestParser;
	delete socketList;
	close(sockfd);
	freeaddrinfo(servinfo);

	return 0;
}

void BuildSocketList()
{
	State state;
	int fd;
	FD_ZERO(&r_fds);
	FD_ZERO(&w_fds);

	// loop through all sockets in the list and
	// add them to the appropriate read or write set
	while(socketList->GetNext(&fd, &state ) == true)
	{
		if(state == OPEN || state == READING)
		{
			FD_SET(fd, &r_fds);	
			if(fd > highfd)
			{	
				highfd = fd;
			}
		
		}
		else if(state == WRITING)
		{
			FD_SET(fd, &w_fds);
			if(fd > highfd)
			{
				highfd = fd;
			}
		}
	}

	// reset the list
	socketList->ResetCurrent();

	return;
}

void HandleSockets()
{
	char buffer[4096];
	int fd = 0;
	ssize_t bsent = 0;
	ssize_t breceived = 0;
	uint offset = 0;
	State state;

	// make sure we are reading from the beginning of the list
	socketList->ResetCurrent();

	// loop throuh our list of sockets and see if any of them are ready as
	// returned from select
	while(socketList->GetNext(&fd, &state) == true)
	{
		// if anything came in on the open-listening socket
		// that means we have a new connection and we need to call
		// accept to accept the new connection and make it ready for reading
		if(state == OPEN && FD_ISSET(fd, &r_fds))
		{
			sockaddr_storage their_addr;
			socklen_t addr_size = sizeof their_addr;
			int nc = accept(fd, (sockaddr*)&their_addr, &addr_size);
			if(nc == -1)
			{
				printf("\n Unable to accept new connection: %s\n", strerror(errno));
				raise(SIGINT);
			}

			numSocks++;
			SetNonBlocking(nc);	

			socketList->AddSocket(nc, READING);
		}
		// call recv on sockets that are ready to be ready from
		else if(state == READING && FD_ISSET(fd, &r_fds))
		{
			printf("Read begin\n");
			breceived = recv(fd, buffer, 4096, 0);
			printf("Read end\n");
			
			if(breceived <= 0)
			{
				printf("Error on read - closing socket %d\n", fd);
				close(fd);
				socketList->SetSocketState(fd, CLOSED);
			}
			else
			{
				bool bValid = socketList->AddRequestToBuffer(fd, buffer, breceived);			
				if(bValid)
				{
					ResourceType rt = httpRequestParser->ParseRequest(buffer);
					socketList->SetResourceType(rt);
					socketList->SetSocketState(fd, WRITING);
				}
			}
		}
		// write responses to sockets that are ready to be written tp
		else if(state == WRITING && FD_ISSET(fd, &w_fds))
		{
			long rlen = 0;
			char* response = NULL;
			
			// parse request
			switch(socketList->GetCurrentType())
			{
				case A2:

					response = httpResponseHelper->GetA2Response(&rlen);
					break;

				case DRACULA:

					response = httpResponseHelper->GetDraculaResponse(&rlen);

					break;

				case MOUNTAIN:

					response = httpResponseHelper->GetMountainResponse(&rlen);

					break;

				case UNKNOWN:
		
					response = httpResponseHelper->GetErrorResponse(&rlen);

					break;
			}
			
			// send the response
			offset = socketList->GetSocketWriteOffset(fd);

			printf("Send start\n");
			bsent = send(fd, response+offset, rlen-offset, 0);
			printf("Send end\n");
			if(bsent == -1)
			{
				printf("\n Unable to send response: %s\n\n", strerror(errno));
				close(fd);
				socketList->SetSocketState(fd, CLOSED);	
			}
			else
			{
				printf(" (%ld of %ld bytes sent)\n\n", bsent+offset, rlen);
				// track how much of the response got sent so we know how much to
				// send next time
				if(bsent+offset < rlen)
				{
					socketList->SetSocketWriteOffset(fd, bsent);
				}
				else
				{
					close(fd);
					socketList->SetSocketState(fd, CLOSED);	
				}
			}
		}
	}
}

void SetNonBlocking(int socket)
{
	int opts = fcntl(socket, F_GETFL);
	opts = (opts | O_NONBLOCK);
	int status = fcntl(socket, F_SETFL, opts);
	if(status == -1)
	{
		printf("\nUnable to set socket in non-blocking mode: %s\n", strerror(errno));
		raise(SIGINT);
	}
}

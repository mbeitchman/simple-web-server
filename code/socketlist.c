// Marc Beitchman
// linked list to hold socket state for
// event driven server
// project 2 - csep551

#include "socketlist.h"
#include <sys/unistd.h>

SocketNode::SocketNode(int _fd, State _state)
{
	fd = _fd;
	state = _state;
	nextSocket = NULL;
	rt = UNKNOWN;
	writeOffset = 0;
	pRequestBuffer = new HTTPRequestBuffer();
}

SocketNode::~SocketNode()
{
	delete pRequestBuffer;
}

SocketNodeList::SocketNodeList()
{
	head = NULL;
	current = NULL;
}

SocketNodeList::~SocketNodeList()
{
	DeleteList();
}

void SocketNodeList::DeleteList()
{
	if(head == NULL)
	{
		return;
	}

	SocketNode* current = head;
	SocketNode* next = current;

	while(current != NULL)
	{
		next = current->nextSocket;
		if(current->state != CLOSED)
		{
			close(current->fd);
		}
		delete current;
		current = next;
	}
}

void SocketNodeList::SetSocketState(int fd, State state)
{
	SocketNode* temp = head;
	while(temp != NULL)
	{
		if(temp->fd == fd)
		{
			temp->state = state;
			break;
		}

		temp = temp->nextSocket;
	}

}


bool SocketNodeList::AddRequestToBuffer(int fd, char* buffer, uint bsize)
{
	SocketNode* temp = head;
	bool valid = false;
	while(temp != NULL)
	{
		if(temp->fd == fd && temp->state == READING)
		{
			temp->pRequestBuffer->AddData(buffer, bsize, &valid);
			return valid;
		}

		temp = temp->nextSocket;
	}

	return valid;
}

void SocketNodeList::SetSocketWriteOffset(int fd, uint writeOffset)
{
	SocketNode* temp = head;
	while(temp != NULL)
	{
		if(temp->fd == fd && temp->state == WRITING)
		{
			temp->writeOffset += writeOffset;
			break;
		}

		temp = temp->nextSocket;
	}

}

uint SocketNodeList::GetSocketWriteOffset(int fd)
{
	SocketNode* temp = head;
	while(temp != NULL)
	{
		if(temp->fd == fd && temp->state == WRITING)
		{
			return temp->writeOffset;
		}

		temp = temp->nextSocket;
	}

	return 0;
}

int SocketNodeList::AddSocket(int fd, State state)
{
	// check if fd is valid
	SocketNode* newNode = new SocketNode(fd, state);

	if(head == NULL)
	{
		head = newNode;
		current = head;
	}
	// add to the end of the list
	else
	{
		newNode->nextSocket = head;
		head = newNode; 
	}
	return 0;
}

bool SocketNodeList::GetNext(int* fd, State* state)
{

	if(current == NULL)
	{
		return false;
	}
		
	// check params
	*fd = current->fd;
	*state = current->state;
	
	current = current->nextSocket;

	return true;
}

void SocketNodeList::ResetCurrent()
{
	current = head;
}

void SocketNodeList::Print()
{
	SocketNode* temp = head;
	while(temp != NULL)
	{
		printf("\nFD in list is %d\n", temp->fd);
		temp = temp->nextSocket;
	}
}

ResourceType SocketNodeList::GetCurrentType()
{
	if(current != NULL)
	{
		return current->rt;
	}

	return UNKNOWN;
}

void SocketNodeList::SetResourceType(ResourceType _rt)
{
	if(current != NULL)
	{
		current->rt = _rt;
	}	
}
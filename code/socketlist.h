// Marc Beitchman
// linked list to hold socket state for
// event driven server
// project 2 - csep551

#include <stdio.h>
#include <stdlib.h>
#include "common.h"

// soocket state
enum State
{
	OPEN = 0, // only initial listening socket can be in this state
	READING = 1, // socket had data to be read
	WRITING = 2, // data can be sent on the socket
	CLOSED = 3 // socket is closed
};

// this class represents an individual socket which contains the following
// 1) the file descriptor
// 2) socket state
// 3) the requested resource type
// 4) the request itself
// 5) the current offset into the request which represent how much data has been sent
class SocketNode
{
	public:
		SocketNode(int _fd, State _state);
		~SocketNode();		

		int fd;
		State state;
		ResourceType rt;
		uint writeOffset;
		HTTPRequestBuffer* pRequestBuffer;
		SocketNode* nextSocket;
};

// linked list of socket nodes
class SocketNodeList
{
	public:
		SocketNodeList();
		~SocketNodeList();
		int AddSocket(int fd, State state);
		void SetSocketState(int fd, State state);
		void ResetCurrent();
		bool GetNext(int* fd, State* state);
		ResourceType GetCurrentType();
		void SetResourceType(ResourceType _rt);
		void SetSocketWriteOffset(int fd, uint writeOffset);
		uint GetSocketWriteOffset(int fd);
		bool AddRequestToBuffer(int fd, char* buffer, uint bsize);
		void Print();

	private:
		void DeleteList();

	private:
		SocketNode* head;
		SocketNode* current;

};
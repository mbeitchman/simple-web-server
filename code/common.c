// Marc Beitchman
// common code for servers
// project 2 - csep551

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

HTTPResponseHelper::HTTPResponseHelper()
{
	InitializeA2();
	InitializeMountain();
	InitializeError();
	InitializeDracula();
}

HTTPResponseHelper::~HTTPResponseHelper()
{
	if(draculaResponse != NULL)
	{
		free(draculaResponse);
	}

	if(a2Response != NULL)
	{
		free(a2Response);
	}

	if(mountainResponse != NULL)
	{
		free(mountainResponse);
	}

	if(errorResponse != NULL)
	{
		free(errorResponse);
	}
}

void HTTPResponseHelper::InitializeA2()
{
	FILE* pFile = fopen("a2.html", "rb");

	// get file size
	fseek(pFile, 0, SEEK_END);
	long fsize = ftell(pFile);
	rewind(pFile);

	// read the file contents into a buffer
	char* buffer = (char*)malloc(sizeof(char)*(fsize));
	fread(buffer, 1, fsize, pFile);

	// get the header size
	long hsize = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

	// allocate the response buffer
	a2ResponseLength = hsize + fsize;
	a2Response = (char*)malloc(sizeof(char)*(a2ResponseLength));

	// copy header into result buffer
	sprintf(a2Response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	memcpy(a2Response+hsize, buffer, fsize);	

	free(buffer);
	fclose(pFile);
}

void HTTPResponseHelper::InitializeDracula()
{
	FILE* pFile = fopen("dracula.html", "rb");

	// get file size
	fseek(pFile, 0, SEEK_END);
	long fsize = ftell(pFile);
	rewind(pFile);

	// read the file contents into a buffer
	char* buffer = (char*)malloc(sizeof(char)*(fsize));
	fread(buffer, 1, fsize, pFile);

	// get the header size
	long hsize = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");

	// allocate the response buffer
	draculaResponseLength = hsize + fsize;
	draculaResponse = (char*)malloc(sizeof(char)*(draculaResponseLength));

	// copy header into result buffer
	sprintf(draculaResponse, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	memcpy(draculaResponse+hsize, buffer, fsize);	

	free(buffer);
	fclose(pFile);
}

void HTTPResponseHelper::InitializeMountain()
{
	FILE* pFile = fopen("mountain.jpg", "rb");

	// get file size
	fseek(pFile, 0, SEEK_END);
	long fsize = ftell(pFile);
	rewind(pFile);

	// read the file contents into a buffer
	char* buffer = (char*)malloc(sizeof(char)*(fsize));
	fread(buffer, 1, fsize, pFile);

	// get the header size
	long hsize = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");

	// allocate the response buffer
	mountainResponseLength = hsize + fsize;
	mountainResponse = (char*)malloc(sizeof(char)*(mountainResponseLength));

	// copy header into result buffer
	sprintf(mountainResponse, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
	memcpy(mountainResponse+hsize, buffer, fsize);	

	free(buffer);
	fclose(pFile);
}

void HTTPResponseHelper::InitializeError()
{
	errorResponseLength = snprintf(NULL, 0, "%s","HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<html>requested page not found</html>");
	errorResponse = (char*)malloc(sizeof(char)*errorResponseLength+1);
	sprintf(errorResponse, "%s", "HTTP/1.1 404 NOT FOUND\r\nContent-Type: text/html\r\n\r\n<html>requested page not found</html>");
	return;
}

char* HTTPResponseHelper::GetDraculaResponse(long* length)
{
	if(length != NULL)
	{
		*length = draculaResponseLength;
	}

	return draculaResponse;
}

char* HTTPResponseHelper::GetA2Response(long* length)
{
	if(length != NULL)
	{
		*length = a2ResponseLength;
	}

	return a2Response;
}

char* HTTPResponseHelper::GetMountainResponse(long* length)
{
	if(length != NULL)
	{
		*length = mountainResponseLength;
	}

	return mountainResponse;
}

char* HTTPResponseHelper::GetErrorResponse(long* length)
{
	if(length != NULL)
	{
		*length = errorResponseLength;
	}

	return errorResponse;
}


HTTPRequestParser::HTTPRequestParser()
{
}

HTTPRequestParser::~HTTPRequestParser()
{
}

ResourceType HTTPRequestParser::ParseRequest(char* request)
{
	if(request == NULL)
	{
		return UNKNOWN;
	}
	
	ResourceType rt = UNKNOWN;
	bool getParsed = false;

	// parse the request
	char* pch = strtok(request, " ");
	while(pch != NULL)
	{
		if(0 == strcasecmp(pch, "GET"))
		{
			getParsed = true;
		}

		pch = strtok(NULL, " ");
		if(getParsed == true && pch != NULL)
		{
			if(0 == strcasecmp(pch, "/a2.html"))
			{
				rt = A2;
				break;
			}	
			else if(0 == strcasecmp(pch, "/book/dracula.html"))
			{
				rt = DRACULA;
				break;
			}
			else if(0 == strcasecmp(pch, "/a2/mountain.jpg"))
			{
				rt = MOUNTAIN;
				break;
			}			
		}
	}

	return rt;
}


HTTPRequestBuffer::HTTPRequestBuffer()
{
	requestBuffer = NULL;
	requestBufferSize = 0;
}

HTTPRequestBuffer::~HTTPRequestBuffer()
{
	if(requestBuffer != NULL)
	{
		free(requestBuffer);
	}
}

uint HTTPRequestBuffer::AddData(char* buffer, uint bsize, bool* isValid)
{
	if(bsize == 0 || isValid == NULL)
	{
		return 0;
	}

	if(requestBuffer == NULL)
	{
		requestBuffer = (char*)malloc(sizeof(char)*bsize);
		memcpy(requestBuffer, buffer, bsize);
	}
	else
	{
		requestBuffer = (char*)realloc(requestBuffer, requestBufferSize+bsize);
		memcpy(requestBuffer, buffer, bsize);
	}

	requestBufferSize += bsize;

	*isValid = IsValidRequest();

	return bsize;

}
	
bool HTTPRequestBuffer::IsValidRequest()
{
	char* pch = strstr(requestBuffer, "\r\n\r\n");

	if(pch != NULL)
	{
		return true;
	}

	return false;
}

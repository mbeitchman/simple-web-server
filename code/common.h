// Marc Beitchman
// common code for servers
// project 2 - csep551

// defines the resources that 
// the server knows about
enum ResourceType
{
	UNKNOWN = 0,
	A2 = 1,
	DRACULA = 2,
	MOUNTAIN = 3,
};

// helper class for generating http responses
class HTTPResponseHelper
{
// methods
public:
	HTTPResponseHelper();
	~HTTPResponseHelper();
	char* GetDraculaResponse(long* length);
	char* GetA2Response(long* length);
	char* GetMountainResponse(long* length);
	char* GetErrorResponse(long* length);

private:
	void InitializeA2();
	void InitializeDracula();
	void InitializeError(); 
	void InitializeMountain();
	
//data
private:
	char* draculaResponse;
	long draculaResponseLength;

	char* a2Response;
	long a2ResponseLength;
	
	char* mountainResponse;
	long mountainResponseLength;

	char* errorResponse;
	long errorResponseLength;
};

// helper class for parsing http requests
class HTTPRequestParser
{
//methods
public:
	HTTPRequestParser();
	~HTTPRequestParser();
	
	ResourceType ParseRequest(char* request);
};

// helper class for buffering http requests and
// determining of the request is valid
class HTTPRequestBuffer
{
// methods
public:
	HTTPRequestBuffer();
	~HTTPRequestBuffer();

	uint AddData(char* buffer, uint bsize, bool* isValid);
private:
	bool IsValidRequest();

// data 
private:
	char* requestBuffer;
	uint requestBufferSize;
};





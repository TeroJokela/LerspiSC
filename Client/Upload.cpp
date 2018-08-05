#include "Includes.h"


bool Upload::initialize()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);

	// WSAStartup needs to be called because we're on Windows
	int iWSAStartup = WSAStartup(wVersionRequested, &wsaData);
	if (iWSAStartup)
	{
#ifdef DEBUG
		oLog->writeLog("WSAStartup() - failed... Error code: " + std::to_string(iWSAStartup));
#endif // DEBUG

		return false;
	}
	
	// Create the socket
	sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sSocket == INVALID_SOCKET)
	{
#ifdef DEBUG
		oLog->writeLog("socket() - failed... WSAGetLastError: " + std::to_string(WSAGetLastError()));
#endif // DEBUG

		return false;
	}

	sa_inServer.sin_family = AF_INET;
	sa_inServer.sin_port = htons(PORT);

	// Store the IP address to our sa_inServer object
	if (InetPtonW(AF_INET, IP, &sa_inServer.sin_addr) != 1)
	{
#ifdef DEBUG
		oLog->writeLog("InetPtonW() - failed... WSAGetLastError: " + std::to_string(WSAGetLastError()));
#endif // DEBUG

		return false;
	}

	// Start the connection
	if (connect(sSocket, (sockaddr*)&sa_inServer, sizeof(sa_inServer)) == SOCKET_ERROR)
	{
#ifdef DEBUG
		oLog->writeLog("connect() - failed... WSAGetLastError: " + std::to_string(WSAGetLastError()));
#endif // DEBUG

		return false;
	}

	return true;
}


void Upload::terminate()
{
	closesocket(sSocket);
}


bool Upload::dataTransfer(std::string strData, std::string &strReturn)
{
	// Calculate the total size that we need to send (we will use this size for memory allocation as well)
	int iTotalSize = sizeof(int) + strData.length();

	// Allocate the memory with the correct size
	char * cBuffer = new char[iTotalSize];

	// Build the whole message
	memcpy(cBuffer, &iTotalSize, sizeof(int));
	memcpy(cBuffer + sizeof(int), strData.c_str(), strData.length());

	// Send the data
	int iSend = send(sSocket, cBuffer, iTotalSize, 0);

	// Free memory
	delete cBuffer;

	if (iSend == SOCKET_ERROR)
	{
#ifdef DEBUG
		oLog->writeLog("send() - failed... WSAGetLastError: " + std::to_string(WSAGetLastError()));
#endif // DEBUG
	
		return false;
	}

	// This will hold the filename
	char Buffer[1024];

	// Get the response from the server and the bytes on how long it is
	// (we can just recv once because it's never going to be over 1024 bytes)
	int iRecvSize = recv(sSocket, &Buffer[0], sizeof(Buffer), 0);

	if (!iRecvSize)
	{
#ifdef DEBUG
		oLog->writeLog("recv() - failed... WSAGetLastError: " + std::to_string(WSAGetLastError()));
#endif // DEBUG

		return false;
	}

	// Allocate the memory for the answer
	strReturn = std::string(Buffer, iRecvSize);

	// Did the server give an error?
	if (strReturn.substr(0, 5) == "Error")
	{
#ifdef DEBUG
		oLog->writeLog("Server gave an error... Error: " + strReturn);
#endif // DEBUG

		return false;
	}

#ifdef DEBUG
	oLog->writeLog("Server response: " + strReturn);
#endif // DEBUG

	return true;
}


Upload oUp;
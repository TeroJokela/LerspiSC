#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <thread>
#include <random>
#include <string.h>


#define PORT 1337


std::default_random_engine gen;
const int iBufferSize = 1024;


void saveFile(int iConnection, sockaddr_in sa_inConnection)
{
	// Store the bytes for the picture here
	std::string pictureBytes;

	// Store the temporary data here
	char cTempBuffer[iBufferSize];

	int iBytesRead = 0;
	int iMessageSize = -1;

	// Read the data until it's completed
	for (;;)
	{
		// Get the data from the client
		int iReadSize = (int)recv(iConnection, &cTempBuffer, (size_t)iBufferSize, 0);
		
		// Did we get any data?
		if (iReadSize > 0)
		{
			// Add the bytes to our "pictureBytes" (we'll strip down the sMessageHeader later)
			pictureBytes.append(cTempBuffer, iReadSize);

			// Keep track of how many bytes we've read so we know when to quit
			iBytesRead += (int)iReadSize;
			
			// Get the total size of the incoming data
			if (iMessageSize == -1)
			{
				// Sorry for this one-liner <3
				iMessageSize = int((unsigned char)(cTempBuffer[0]) << 24 |
								   (unsigned char)(cTempBuffer[1]) << 16 |
								   (unsigned char)(cTempBuffer[2]) << 8 |
								   (unsigned char)(cTempBuffer[3]));
			}
		}
		
		// Check if we got any data / if there's any more data to be read
		if (iReadSize <= 0 || iBytesRead >= iMessageSize)
			break;
	}

	// Get the IP address
	std::string strIp = inet_ntoa(sa_inConnection.sin_addr);

	printf("%i: IP: '%s'\n", iConnection, strIp.c_str());

	// Remove the message size from the message
	pictureBytes.erase(0, sizeof(int));

	// Make our random filename
	std::uniform_int_distribution<int> randomNumber(49, 57);
	std::uniform_int_distribution<int> randomChar(65, 90);
	std::uniform_int_distribution<int> numberOrChar(0, 1);
	std::string strFilename;
	for (int i = 0; i < 21; i++)
		strFilename += (char)((numberOrChar(gen)) ? randomChar(gen) : randomNumber(gen));
	strFilename += ".png";

	// This will be used when making the file
	std::string strPath = "PATH TO DIRECTORY";
	
	// Write the bytes to a file
	std::ofstream file(strPath + strFilename, std::ios::out | std::ios::binary);
	if (file.is_open())
	{
		file.write(&pictureBytes[0], pictureBytes.length());
		file.close();
	}

	std::string strMessage = "Saved as: \"" + strFilename + "\" to \"" + strPath + "\"!";

	// Send back the message
	send(iConnection, strPath.c_str(), strPath.length(), 0);

	// Close the connection
	close(iConnection);

	printf("%i: Picture saved as: '%s'\n", iConnection, strFilename.c_str());
}


int main()
{
	gen.seed(clock());

	// Create our socket file descriptor
	int iServerFD = socket(AF_INET, SOCK_STREAM, 0);
	if (iServerFD == 0)
	{
		perror("socket() - failed");
		return 0;
	}

	int iOptVal = 1;

	if (setsockopt(iServerFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &iOptVal, sizeof(iOptVal)))
	{
		perror("setsockopt() - failed");
		return 0;
	}

	sockaddr_in sa_inAddress;
	sa_inAddress.sin_family = AF_INET;
	sa_inAddress.sin_addr.s_addr = INADDR_ANY; // Accept all incoming messages
	sa_inAddress.sin_port = htons(PORT); // Set the port

	// Forcefully attach the sockt to the port (we use a loop because if you close a program
	// in while testing, all the connections need to be closed before the bind is successful
	while (bind(iServerFD, (sockaddr*)&sa_inAddress, sizeof(sa_inAddress)))
		sleep(1);

	// Prepare to accept connections
	if (listen(iServerFD, 3))
	{
		perror("listen() - failed");
		return 0;
	}

	socklen_t sl_tAddrLen = sizeof(sockaddr_in);

	printf("Initialization success!\n");

	// Start accepting new connections here
	for (;;)
	{
		printf("Waiting for a new connection...\n");

		// Store the socket information for our new connection here
		sockaddr_in sa_inNewConnection;

		// accept() waits for a new connection
		int iNewConnection = accept(iServerFD, (sockaddr*)&sa_inNewConnection, &sl_tAddrLen);
		
		// Did something go wrong?
		if (iNewConnection == -1)
		{
			printf("accept() - failed\n");
			continue;
		}

		printf("We got a new connection! [%i]\n", iNewConnection);

		// Create a new thread to handle the connection
		std::thread saveFileThread(saveFile, iNewConnection, sa_inNewConnection);
		saveFileThread.detach();
	}

	return 0;
}
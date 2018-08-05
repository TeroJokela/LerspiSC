#pragma once


#define IP L"IP HERE"
#define PORT 1337


class Upload
{
public:
	bool initialize();
	void terminate();
	bool dataTransfer(std::string strData, std::string &strReturn);

private:
	SOCKET sSocket;
	sockaddr_in sa_inServer;
};


extern Upload oUp;
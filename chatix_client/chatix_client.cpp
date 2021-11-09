#include <iostream>
#pragma warning(disable : 4996)

/*
	Create a TCP socket
*/

#include<stdio.h>
#include<winsock2.h>
#include <string>
#include <sstream>
#include <process.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

DWORD WINAPI request(LPVOID lpParameter);

int main(int argc, char* argv[])
{
	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	std::string ip_add;
	int port;

	std::cout << "please input ip address of server" << std::endl;
	std::cin >> ip_add;
	std::cout << std::endl;
	std::cout << "please input port of server" << std::endl;
	std::cin >> port;
	std::cout << std::endl;

	server.sin_addr.s_addr = inet_addr(ip_add.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to remote server
	if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");

	//Send some data
	DWORD dwThreadId;
	CreateThread(NULL, 0, request, (LPVOID)s, 0, &dwThreadId);
	
	while (true)
	{
		std::string message_tmp;
		std::getline(std::cin, message_tmp);
		std::stringstream message;
		message << message_tmp << "/end";
		if (send(s, message.str().c_str(), message.str().length(), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
	}
	

	return 0;
}


DWORD WINAPI request(LPVOID lpParameter)
{
	std::string message;
	SOCKET AcceptSocket = (SOCKET)lpParameter;
	int client_socket = (int)lpParameter;

	const int max_client_buffer_size = 1024;
	
	int n = 0;

	while (true)
	{
		char buf[max_client_buffer_size];
		if (recv(AcceptSocket, buf, max_client_buffer_size, 0) < 0)
		{
			return 0;
		}
		if (strstr(buf, "/end") != NULL)
		{
			//message = strstr(buf, "/end");
			message = buf;
			int num = message.find("/end");
			message = message.substr(0, num);
			std::cout << "> " << message << std::endl;
		}

	}


}
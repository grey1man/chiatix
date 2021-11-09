#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <process.h>

#define _WIN32_WINNT 0x501

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable : 4996)

std::string message;
int listen_socket;
struct addrinfo* addr;
std::vector<int> clients;

DWORD WINAPI request(LPVOID lpParameter);
DWORD WINAPI response(LPVOID lpParameter);


int init_socket(PCSTR ip_address, PCSTR port)
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << "\n";
		return result;
	}

	//struct addrinfo* addr = NULL;

	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	result = getaddrinfo(ip_address, port, &hints, &addr);

	if (result != 0) {
		std::cerr << "getaddrinfo failed: " << result << "\n";
		WSACleanup(); // выгрузка библиотеки Ws2_32.dll
		return 1;
	}

	listen_socket = socket(addr->ai_family, addr->ai_socktype,
		addr->ai_protocol);

	if (listen_socket == INVALID_SOCKET) {
		std::cerr << "Error at socket: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		WSACleanup();
		return 1;
	}
}

int bind_socket()
{
	// Привязываем сокет к IP-адресу
	int result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);

	// Если привязать адрес к сокету не удалось, то выводим сообщение
	// об ошибке, освобождаем память, выделенную под структуру addr.
	// и закрываем открытый сокет.
	// Выгружаем DLL-библиотеку из памяти и закрываем программу.
	if (result == SOCKET_ERROR) {
		std::cerr << "bind failed with error: " << WSAGetLastError() << "\n";
		freeaddrinfo(addr);
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}
}

int listen_()
{
	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "listen failed with error: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}
}

int accept_()
{
	bool flag_in_clients = false;
	sockaddr_in cs_addr; //или SOCKADDR_IN cs_addr;
	socklen_t cs_addrsize = sizeof(cs_addr); //или int cs_addrsize = sizeof(cs_addr);
	int client_socket = accept(listen_socket, (struct sockaddr*)&cs_addr, &cs_addrsize);

	DWORD dwThreadId;
	for (int i = 0; i < clients.size(); i++)
	{
		if (client_socket == clients.at(i))
			flag_in_clients = true;
	}
	clients.push_back(client_socket);

	if (not(flag_in_clients))
		CreateThread(NULL, 0, request, (LPVOID)client_socket, 0, &dwThreadId);
	//CreateThread(NULL, 0, response, (LPVOID)client_socket, 0, &dwThreadId);

	char* client_addres = inet_ntoa(cs_addr.sin_addr);
	//CreateThread(NULL, 0, ProcessClient, (LPVOID)AcceptSocket, 0, &dwThreadId);

	if (client_socket == INVALID_SOCKET) {
		std::cerr << "accept failed: " << WSAGetLastError() << "\n";
		closesocket(listen_socket);
		WSACleanup();
		return 1;
	}
}

DWORD WINAPI request(LPVOID lpParameter)
{
	std::cout << std::endl;
	std::cout << "new request thread started" << std::endl;
	SOCKET AcceptSocket = (SOCKET)lpParameter;
	int client_socket = (int)lpParameter;
	
	const int max_client_buffer_size = 1024;
	int n = 0;

	while (true)
	{
		char buf[max_client_buffer_size] = {0};
		recv(AcceptSocket, buf, max_client_buffer_size, 0);
		if (strstr(buf, "/end") != NULL)
		{
			//message = strstr(buf, "/end");
			message = buf;
			int num = message.find("/end");
			message = message.substr(0, num);
			message.append("/end");
			for (int i = 0; i < clients.size(); i++)
				if (clients.at(i) != client_socket)
					send((SOCKET)clients.at(i), message.c_str(), message.length(), 0);
		}
	}
	
	
}

DWORD WINAPI response(LPVOID lpParameter)
{
	std::cout << std::endl;
	std::cout << "new response thread started" << std::endl;
	SOCKET AcceptSocket = (SOCKET)lpParameter;
	std::string msg_last = message;
	while (true)
	{
		if (msg_last != message)
			send(AcceptSocket, message.c_str(), message.length(), 0);
	}
}




int main()
{
	std::string ip_add;
	std::string port;

	std::cout << "please input ip address of this sever" << std::endl;
	std::cin >> ip_add;
	std::cout << "please input port of this sever" << std::endl;
	std::cin >> port;

	init_socket(ip_add.c_str(), port.c_str());
	bind_socket();
	listen_();

	while (true)
		accept_();
}


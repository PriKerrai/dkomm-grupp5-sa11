////  Server.cpp : Defines the entry point for the console application.
////
//
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <time.h>

int _tmain(int argc, _TCHAR* argv[])
{
	// Initiera WinSock
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );

	// Skapa en socket
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	// Ta reda p� addressen till localhost:4567
	struct addrinfo *info;
    int ok = getaddrinfo("localhost","4567",NULL,&info);
	if(ok!=0) {
		WCHAR * error = gai_strerror(ok);
		printf("%s\n",error);
	}
	else while(info->ai_family != AF_INET && info->ai_next != NULL)
		info = info->ai_next;


	// Lyssna p� addressen via socketen
	ok = bind(s,info->ai_addr,info->ai_addrlen);
	if(ok == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("%d\n",err);
	}
	ok = listen(s,SOMAXCONN);
	if(ok == SOCKET_ERROR) {
		int err = WSAGetLastError();
		printf("%d\n",err);
	}

	// V�nta p� inkommande anrop
	struct sockaddr clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	SOCKET s1 = accept(s,&clientAddr,&clientAddrLen);
	if(s1 != INVALID_SOCKET) {
		char hostName[100];
		char portName[100];
		int ok = getnameinfo(&clientAddr,clientAddrLen,hostName,100,portName,100,NI_NUMERICSERV );
		if(ok == 0) {
			printf("Accept incoming from: %s at port %s\n",hostName,portName);
		}

		// Skriv ut meddelandet fr�n klienten
		int iResult;
		char *inputHTTP = (char*) malloc(sizeof(char)*512);
		iResult = recv(s1, inputHTTP, 512, 0);
		fwrite(inputHTTP,1,iResult,stdout);
		fflush(stdout);
		char cmdHTTP[80];
		char filenameHTTP[80];
		char protocolHTTP[80];
		ok = sscanf(inputHTTP,"%s %s %s",cmdHTTP,filenameHTTP,protocolHTTP);
		// Skicka tillbaka ett svar till klienten
		char *message = "HTTP/1.1 200 OK\nDate: Thu, 19 Feb 2009 12:27:04 GMT\nServer: Apache/2.2.3\nLast-Modified: Wed, 18 Jun 2003 16:05:58 GMT\nETag: \"56d-9989200-1132c580\"\nContent-Type: text/html\nContent-Length: 15\nAccept-Ranges: bytes\nConnection: close\n\n<html><head><title>hej</title></head><body><h1>hej</h1></body></html>";
		int len = send(s1,message,strlen(message), 0);

		// St�ng sockets
		closesocket(s1);
	}
	else {
		int err = WSAGetLastError();
		printf("%d\n",err);
	}
	closesocket(s);

	// Deinitiera WinSock
	WSACleanup();
	return 0;
}


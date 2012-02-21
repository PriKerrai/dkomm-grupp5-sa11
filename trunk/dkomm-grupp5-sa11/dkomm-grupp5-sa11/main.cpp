//  Server.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "winsock2.h"
#include "ws2tcpip.h"

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

	// Ta reda på addressen till localhost:4567
	struct addrinfo *info;
    int ok = getaddrinfo("localhost","4567",NULL,&info);
	if(ok!=0) {
		WCHAR * error = gai_strerror(ok);
		printf("%s\n",error);
	}
	else while(info->ai_family != AF_INET && info->ai_next != NULL)
		info = info->ai_next;


	// Lyssna på addressen via socketen
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

	// Vänta på inkommande anrop
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

		// Skriv ut meddelandet från klienten
		int iResult;
		char inputHTTP[512]; //= (char*) malloc(sizeof(char)*512);
		iResult = recv(s1, inputHTTP, 512, 0);
		fwrite(inputHTTP,1,iResult,stdout);
		fflush(stdout);
		char cmdHTTP[80];
		char filenameHTTP[80];
		char protocolHTTP[80];
		ok = sscanf(inputHTTP,"%s %s %s",cmdHTTP,filenameHTTP,protocolHTTP);
		char *next = strstr(inputHTTP,"\n");
		while(next) {
			char name[80];
			char value[80];
			ok = sscanf(next+1,"%s",name);
			char *space = strstr(next+1," ");
			next = strstr(next+1,"\n");
			if(space) {
				memcpy(value,space+1,next-space-1);
				value[next-space-1]=0;
			}
			if(ok==1) {
				if(strcmp(name,"If-modified-since:")==0) {
					if(strcmp(cmdHTTP,"GET")==0) {
						printf("I got a conditional GET with the date: %s\n",value);
					}
				}
			}
		}
		// Skicka tillbaka ett svar till klienten
		char *message = "Message recieved by server.";
		int len = send(s1,message,strlen(message),0);

		// Stäng sockets
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
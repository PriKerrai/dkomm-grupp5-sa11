////  Server.cpp : Defines the entry point for the console application.
////
//
#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <time.h>
#include <simpio.h>
#include <genlib.h>
#include "F:\Dokument\libs\strlib.h"



char *loadHtml(char *filename);

int _tmain(int argc, _TCHAR* argv[]){
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
		char *inputHTTP = (char*) malloc(sizeof(char)*512);
		iResult = recv(s1, inputHTTP, 512, 0);
		fwrite(inputHTTP,1,iResult,stdout);
		fflush(stdout);
		char cmdHTTP[80];
		char filenameHTTP[80];
		char protocolHTTP[80];
		ok = sscanf(inputHTTP,"%s %s %s",cmdHTTP,filenameHTTP,protocolHTTP);
		// Skicka tillbaka ett svar till klienten
		string message = "HTTP/1.1 200 OK\n"
						"Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
						"Server: Apache/2.2.3\n"
						"Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
						"ETag: \"56d-9989200-1132c580\"\n"
						"Content-Type: text/html\n"
						"Content-Length: 15\n"
						"Accept-Ranges: bytes\n"
						"Connection: close\n"
						"\n";
		message = Concat(message,loadHtml(filenameHTTP));
		int len = send(s1,message,strlen(message), 1);
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

string loadHtml(char *filename){
	char *line;
	char *fileContent = "";
	FILE *htmlFile;
	htmlFile = fopen(filename,"r");
	if(htmlFile==NULL)
		return "<b>404</b>";
	do{
		line = ReadLine(htmlFile);
		fileContent = Concat(fileContent,line);
	}while(line[0] != '\0');
	return fileContent;
}


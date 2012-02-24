////  Server.cpp : Defines the entry point for the console application.
////
//
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include "winsock2.h"
#include "ws2tcpip.h"
#include <time.h>
#include <fstream>
#include <string>

using namespace std;
string getMime(string filename);
void loadHtml(char *buffer, string filename, int &size);
string filetypeToMime(string filetype);
string toLowerCase(string toConvert);
int _tmain(int argc, _TCHAR* argv[]){
	while(TRUE){
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
		
		int size = 0;
		char buffer[1024];
		loadHtml(buffer,filenameHTTP,size);
		
		
		int bytesIndex = 0;
		//int len = send(s1,message._Myptr(),message.length(), 1);
		while(size != 0){
			int len = send(s1,buffer,size, 1);
			size -= len;
			bytesIndex +=len;
		}
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
	}
	return 0;
	
}

string getFiletype(string filename){
	int i = filename.length();
	int len = i - 1;
	string filetype = "";
	filename = toLowerCase(filename);
	while(filename[i-1] != '.'){
		i--;
	}
	filetype.append(filename.substr(i));
	return filetype;
}
string getMime(string fileName){
	string filetype = getFiletype(fileName);
	int len = filetype.length() -1;
	if(filetype.compare("html") == 0)
		return "text/html";
	else if(filetype.compare("jpeg") == 0||filetype.compare("jpg") == 0)
		return "image/jpeg";
	else if(filetype.compare("png") == 0)
		return "image/png";
	else return "kuk";
}

void loadHtml(char buffer, string filename, int &size){
	int i = 0;
	int len;
	string line;
	string fileContent = "";
	filename.erase(0,1);
	string filetype = getFiletype(filename);
	ifstream htmlFile;
	string mime = getMime(filename);
	char *temp;
	if(filetype.compare("html") == 0){
		htmlFile.open(filename,ios::in);
		if(htmlFile.is_open()){
			do{
				getline(htmlFile, line);
				len = line.length();
				for(int j=0;j<len; j++){
					temp[i] = line[j];
				}
				temp[i+1] = '\n';
				i++;
			}while(htmlFile.good());
		}else
			buffer = "<b>404</b>";
	}else{
		htmlFile.open(filename,ios::binary);
	}
	buffer =			"HTTP/1.1 404 OK\n"
						"Date: Thu, 19 Feb 2012 16:27:04 GMT\n"
						"Server: Apache/2.2.3\n"
						"Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
						"ETag: \"56d-9989200-1132c580\"\n"
						"Content-Type: " + getMime(filename) +
						"\nContent-Length: 15\n"
						"Accept-Ranges: bytes\n"
						"Connection: close\n"
						"\n" + temp;
	size = htmlFile.gcount();
	
}
char *stringToArray();
string toLowerCase(string toConvert){
	string converted = "";
	int len = toConvert.length() - 1;
	for (int i=0;i<len;i++){
		if (toConvert[i] >= 0x41 && toConvert[i] <= 0x5A)
			toConvert[i] = toConvert[i] + 0x20;
	}
	return toConvert;
}
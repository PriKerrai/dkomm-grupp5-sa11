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
#include <string.h>
#include <sys/stat.h>
#include <ctime>

typedef struct{
	char httpPort[150];
	char telnetPort[150];
	char wwwPath[150];
	char password[150];
	char logPath[150];
} configT;

using namespace std;


configT loadCfg();
string getMime(string filename);
string getMonth(int month);
string getWeekday(int weekday);
string createHeader(string filename, ULONG size);
char *loadBin(string filename, ULONG &size);
char *mergeVector(char *vector1, ULONG size1, char *vector2, ULONG size2);
char *loadHtml(string filename, ULONG &size);
string getFiletype(string filename);
string filetypeToMime(string filetype);
string getCurrentDate();
void stringToVector(string toConvert, char vector[], ULONG size);
static bool on = true;
int _tmain(int argc, _TCHAR* argv[]){
	//configT config = loadCfg();
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
	while(on){
	int iResult;
	SOCKET s1 = accept(s,&clientAddr,&clientAddrLen);
	if(s1 != INVALID_SOCKET) {
		char hostName[100];
		char portName[100];
		bool loggedIn = false;
		int ok = getnameinfo(&clientAddr,clientAddrLen,hostName,100,portName,100,NI_NUMERICSERV );
		if(ok == 0) {
			printf("Accept incoming from: %s at port %s\n",hostName,portName);
		}

		// Läsa av 
		string cmd;
		string arg;
		char line[100];
		char *temp = (char *)malloc(1);

		int i = 0;
		while(true){
			while(*temp != '\n'){
				iResult = recv(s1, temp, 1, 0);
				line[i] = *temp;
			i++;
			}
			*temp = ' ';
			//cmd = line;
			//send(s1,cmd._Myptr(),cmd.length(), 1);
			int j = sscanf(line, "%s", cmd.c_str());
			// lägger till ett \0 på slutet =)
			cmd = cmd.c_str();
			int hej = cmd.compare("password");
			if ((cmd.compare("password") == 0) && !loggedIn){
				int j = sscanf(line, "password %s",arg.c_str());
				arg = arg.c_str();
				if(arg.compare("hemligt") == 0){
					loggedIn = true;
					send(s1,"u be h4x0r\n",11, 1);
				}
				else loggedIn = false;
			}
			else if (cmd.compare("status") == 0 && loggedIn){
				if (on)
					send(s1,"auf\n",4,1);
				else
					send(s1,"ab\n",3,1);
			}
			else if (cmd.compare("loggfile") == 0 && loggedIn){
				//fopen;
			}
			else if (cmd.compare("server") == 0 && loggedIn){
				int j = sscanf(line, "server %s",arg.c_str());
				arg = arg.c_str();
				if(arg.compare("off") == 0){
					on = false;
					send(s1,"Turning off\n",12,1);
				}
				else if(arg.compare("on") == 0){
					on = true;
					send(s1,"Turning on\n",11,1);
				}
			}

			i = 0;
			cmd = "\0";
			arg = "\0";
		}
		loggedIn = false;
		closesocket(s1);

		//int i = 0;
		


		// Skriv ut meddelandet från klienten
		/*int iResult;
		char *inputHTTP = (char*) malloc(512);
		iResult = recv(s1, inputHTTP, 512, 0);
		fwrite(inputHTTP,1,iResult,stdout);
		fflush(stdout);
		char cmdHTTP[80];
		char filenameHTTP[80];
		char protocolHTTP[80];
		ok = sscanf(inputHTTP,"%s /%s %s",cmdHTTP,filenameHTTP,protocolHTTP);*/
		// Skicka tillbaka ett svar till klienten
		/*string filetype = getFiletype(filenameHTTP);
		ULONG size = 0;
		ULONG len = 0;
		char *buffer;
		buffer = loadBin(filenameHTTP,size);
		len = send(s1,buffer,size, 1);*/
		
		//free(buffer);
		// Stäng sockets
		closesocket(s1);
		
	}
	else {
		int err = WSAGetLastError();
		printf("%d\n",err);
	}
	}
	closesocket(s);

	// Deinitiera WinSock
	WSACleanup();
	
	return 0;
	
}
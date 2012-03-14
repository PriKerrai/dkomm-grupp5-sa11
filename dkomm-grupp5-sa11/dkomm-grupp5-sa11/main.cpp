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
#include <windows.h>
#include <process.h>
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
void telnet(SOCKET s);
unsigned __stdcall httpCall(SOCKET s);

bool on = true;
int _tmain(int argc, _TCHAR* argv[]){
	configT config = loadCfg();
	// Initiera WinSock
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	
	// Skapa en socket 
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	HANDLE hThread;
    unsigned threadID;

    printf( "Creating second thread...\n" );

    // Create the second thread.
    hThread = (HANDLE)_beginthreadex( NULL, 0, &httpCall, NULL, 0, &threadID );
	printf( "Creating second thread...\n" );

    // Create the second thread.
    hThread = (HANDLE)_beginthreadex( NULL, 0, &httpCall, s, 0, &threadID );

	// Deinitiera WinSock
	WSACleanup();
	
	return 0;
	
}

string getFiletype(string filename){
	int i = filename.length();
	int len = i - 1;
	string filetype = "";
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
	else if(filetype.compare("zip") == 0)
		return "application/zip";
	else if(filetype.compare("ico") == 0)
		return "image/vnd.microsoft.icon";
	else return "other";
}

char *loadBin(string filename, ULONG &size){
	fstream binFile;
	string line;
	string mime;
	ULONG buffsize = 0;
	char *buffer = '\0';
	int headLen = 0; 
	char *headerVector;
	string header;
	binFile.open(filename,ios::in|ios::binary|ios::ate);
	if(binFile.is_open()){
		if(!(binFile.tellg() > sizeof(ULONG)))
			buffsize = 0;
		buffsize = binFile.tellg();
		buffer = (char *) malloc(buffsize);
		header = createHeader(filename, buffsize);
		printf("\n\n\n\n\n%s\n",header._Myptr());
		headLen =header.length(); 
		headerVector = (char *)malloc(headLen);
		headerVector = header._Myptr();
		binFile.seekg (0, ios::beg);
		binFile.read (buffer, buffsize);
		size = buffsize + headLen;
		buffer = mergeVector(headerVector,headLen,buffer,buffsize);
	}else
		size = 0;
	
	return buffer;
}
void stringToVector(string toConvert, char vector[], ULONG size){
	for (ULONG i = 0; i<size; i++){
		vector[i] = toConvert[i];
	}
}
char *mergeVector(char *vector1, ULONG size1, char *vector2, ULONG size2){
	char * newVector = (char *)malloc(size1+size2);
	ULONG i,j;
	for (i = 0; i < size1; i++){
		newVector[i] = vector1[i];
	}
	for ( j = 0; j < size2; j++){
		newVector[i+j] = vector2[j];
	}
	return newVector;

}
string createHeader(string filename, ULONG size){
	struct stat attrib;
	char intToStrBuff[32];
	struct tm* clock;
	string mime = getMime(filename);
	stat(filename.c_str(),&attrib);
	clock = gmtime(&(attrib.st_mtime));
	string temp = asctime(clock);
	string fileDate = temp.substr(0,3)+", " +temp.substr(8,2)+" "+temp.substr(4,3)+" " +temp.substr(20,4)+" " +temp.substr(11,8);
	temp = getCurrentDate();
	string currentDate = temp.substr(0,3)+", " +temp.substr(8,2)+" "+temp.substr(4,3)+" " +temp.substr(20,4)+" " +temp.substr(11,8);
	string ulltoa =  _ultoa(size,intToStrBuff,10);
	return	"HTTP/1.1 200 OK\r\n"
			"Connection: close\r\n"
			"Date: "+currentDate+ " GMT\r\n"+
			"Server: sürver\r\n"
			"Last-Modified: "+fileDate+"\r\n"
			"Content-Length: " + _ultoa(size,intToStrBuff,10) +"\r\n"+
			"Content-Type: " +mime+"\r\n"+
			"\r\n";
}

string getCurrentDate(){
	struct tm* clock;
	time_t timeinfo;
	time(&timeinfo);
	clock = localtime(&timeinfo);
	return asctime(clock);
	
}

configT loadCfg(){
	configT newConfig;
	fstream file;
	int size = 0;
	string line;
	string config = "";
	file.open("config.txt", ios::in|ios::binary);
	while(file.good()){
		getline(file,line);
		size += line.length();
		config.append(line);
		config.append("\n");
	}
	int ok = sscanf(config._Myptr(),
							"httpPort: %s\n"
							"telnetPort: %s\n"
							"wwwPath: %s\n"
							"password: %s\n"
							"logPath: %s",	
							newConfig.httpPort,
							newConfig.telnetPort,
							newConfig.wwwPath,
							newConfig.password,
							newConfig.logPath);

	file.close();
	return newConfig;
}
void telnet(SOCKET s){
	// Ta reda på addressen till localhost:8081
	struct addrinfo *info;
    int ok = getaddrinfo("localhost","8081",NULL,&info);
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
	bool loggedIn = false;
	// Vänta på inkommande anrop
	struct sockaddr clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	while(true)
	{
		int iResult;
		SOCKET s1 = accept(s,&clientAddr,&clientAddrLen);
		if(s1 != INVALID_SOCKET) {
			char hostName[100];
			char portName[100];
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
			send(s1,"Welcome, enter password to log in. press escape to exit\n\r",57, 1);
			while(true){
				while(*temp != '\n'){
					iResult = recv(s1, temp, 1, 0);
					if(*temp == 27 || iResult == 0) //escape stänger telnetklienten och återgår till att lyssna på s1
						break;
					line[i] = *temp;
				i++;
				}
				if(*temp == 27 || iResult == 0) //escape stänger telnetklienten och återgår till att lyssna på s1
						break;
				*temp = ' ';
				int j = sscanf(line, "%s", cmd.c_str());
				// lägger till ett \0 på slutet =)
				cmd = cmd.c_str();
				int hej = cmd.compare("password");
				if ((cmd.compare("password") == 0) && !loggedIn){
					int j = sscanf(line, "password %s",arg.c_str());
					arg = arg.c_str();
					if(arg.compare("hemligt") == 0){
						loggedIn = true;
						send(s1,"You are now logged in\n\r",23, 1);
					}
					else {
						loggedIn = false;
						send(s1,"Invalid password\n\r",18, 1);
					}
				}
				else if (cmd.compare("status") == 0 && loggedIn){
					if (on)
						send(s1,"Server is on\n\r",14,1);
					else
						send(s1,"Server is off\n\r",15,1);
				}
				else if (cmd.compare("loggfile") == 0 && loggedIn){
					FILE *infile;
					char ch;
					string ch2;
	
					infile = fopen("config.txt","r");
					while(infile != NULL)
					{
						ch = fgetc(infile);
						ch2 = ch;
						if(ch != EOF)
						{
							send(s1,ch2._Myptr(), 1, 1);
							if(ch == '\n')
							{
								ch2 = "\r";
								send(s1,ch2._Myptr(), 1, 1);
							}
						}
						else 
						{
							send(s1,"\n\r", 2, 1);
							break;
						}
					}
				}
				else if (cmd.compare("server") == 0 && loggedIn){
					int j = sscanf(line, "server %s",arg.c_str());
					arg = arg.c_str();
					if(arg.compare("off") == 0){
						on = false;
						send(s1,"Webserver is now off\n\r",22,1);
					}
					else if(arg.compare("on") == 0){
						on = true;
						send(s1,"Webserver is now on\n\r",21,1);
					}
					else send(s1,"Invalid arguement (on/off)\n\r",28, 1);
				}
				else if(!loggedIn)
					send(s1, "Enter password first\n\r",22,1);
				else
					send(s1, "Invalid command\n\r",17,1);
				i = 0;
				cmd = "\0";
				arg = "\0";
			}
			loggedIn = false;

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
}
unsigned __stdcall httpCall(SOCKET s){


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
	bool on = true;
	while(on){
	
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
			char *inputHTTP = (char*) malloc(1024);
			iResult = recv(s1, inputHTTP, 1024, 0);
			fwrite(inputHTTP,1,iResult,stdout);
			fflush(stdout);
			char cmdHTTP[80];
			char filenameHTTP[80];
			char protocolHTTP[80];
			ok = sscanf(inputHTTP,"%s /%s %s",cmdHTTP,filenameHTTP,protocolHTTP);
			// Skicka tillbaka ett svar till klienten
			string filetype = getFiletype(filenameHTTP);
			ULONG size = 0;
			ULONG len = 0;
			char *buffer;
			buffer = loadBin(filenameHTTP,size);
			len = send(s1,buffer,size, 1);
		
			free(buffer);
			// Stäng sockets
			closesocket(s1);
		
		}else {
			int err = WSAGetLastError();
			printf("%d\n",err);
		}
	}
	closesocket(s);

}
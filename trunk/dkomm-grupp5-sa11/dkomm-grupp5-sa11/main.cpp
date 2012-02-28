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
	string httpPort;
	string telnetPort;
	string wwwPath;
	string password;
	string logPath;
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
		char *inputHTTP = (char*) malloc(512);
		iResult = recv(s1, inputHTTP, 512, 0);
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
	return	"HTTP/1.1 200 OK\n"
			"Date: "+currentDate+ "\n"+
			"Server: MegaSurver1337\n"
			"Last-Modified: "+fileDate+"\n"
			"Content-Type: " +mime+
			"\nContent-Length: " + _ultoa(size,intToStrBuff,10) + 
			"\nAccept-Ranges: bytes\n"
			"Connection: closed\n"
			"\n";
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
	newConfig.logPath = "hej";
	ifstream file;
	file.open("config.cfg");
	char *config = (char *) malloc(file.tellg());
	file.read(config,file.tellg());
	int ok = sscanf(config,	"httpPort: %s\n"
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

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
using namespace std;
typedef struct{
	string httpPort;
	string telnetPort;
	string wwwPath;
	string password;
	string logPath;
} configT;

typedef struct{
	SOCKET s;
	char *hostname;
}sockInfoT;


configT loadCfg();
string getMime(string filename);
string getMonth(int month);
string getWeekday(int weekday);
string createHeader(string filename, ULONG size, int status);
char *loadBin(string filename, ULONG &size, int status);
char *mergeVector(char *vector1, ULONG size1, char *vector2, ULONG size2);
char *loadHtml(string filename, ULONG &size);
string getFiletype(string filename);
string filetypeToMime(string filetype);
unsigned __stdcall httpThread(void *pArg);
unsigned __stdcall httpMainThread(void *pArg);
void printToLog(string hostname, string cmdHTTP, string filenameHTTP, string protocolHTTP, int status, ULONG fileSize);
string getDate(void);
string fileDate(string filename);
int getStatus(string filename, string date);
unsigned __stdcall telnetThread(void *pArg);

int threads = 0;
bool on = true;
configT config;
int _tmain(int argc, _TCHAR* argv[]){
	config = loadCfg();
	// Initiera WinSock
	HANDLE hThreadHttp;
	HANDLE hThreadTelnet;
	unsigned threadID;
	while (true){	
		hThreadTelnet = (HANDLE) _beginthreadex(NULL, 0, &telnetThread, NULL, 0, &threadID);
		hThreadHttp = (HANDLE) _beginthreadex(NULL, 0, &httpMainThread, NULL, 0, &threadID);
		WaitForSingleObject(hThreadHttp, INFINITE);
		WaitForSingleObject(hThreadTelnet, INFINITE);
	}
	CloseHandle(hThreadHttp);
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
	else if(filetype.compare("gif") == 0)
		return "image/gif";
	else if(filetype.compare("zip") == 0)
		return "application/zip";
	else if(filetype.compare("ico") == 0)
		return "image/vnd.microsoft.icon";
	else return "other";
}

char *loadBin(string filename, ULONG &size, int status){
	fstream binFile;
	string line;
	string mime;
	ULONG buffsize = 0;
	char *buffer = '\0';
	int headLen = 0; 
	char *headerVector;
	string header;
	binFile.open(filename,ios::in|ios::binary|ios::ate);
	size = 0;
	if(!(binFile.tellg() > sizeof(ULONG)))
		buffsize = 0;
	else buffsize = binFile.tellg();
	buffer = (char *) malloc(buffsize);
	header = createHeader(filename, buffsize, status);
	printf("\n\n\n\n\n%s\n",header._Myptr());
	headLen =header.length(); 
	headerVector = (char *)malloc(headLen);
	headerVector = header._Myptr();
	if(status == 200)
	{
	binFile.seekg (0, ios::beg);
	binFile.read (buffer, buffsize);
	
	size = buffsize + headLen;
	buffer = mergeVector(headerVector,headLen,buffer,buffsize);
	return buffer;
	}
	else {
		size = headLen;
		return mergeVector(headerVector,headLen,buffer,buffsize);
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
string createHeader(string filename, ULONG size, int status){
	struct stat attrib;
	char intToStrBuff[32];
	struct tm* clock;
	string mime = getMime(filename);
	stat(filename.c_str(),&attrib);
	string fileDate2 = fileDate(filename);
	string currentDate = getDate();
	string status2 = _itoa(status,intToStrBuff,10);
	return	"HTTP/1.1 "+status2+" OK\r\n"
			"Connection: close\r\n"
			"Date: "+currentDate+ " GMT\r\n"+
			"Server: MegaSurver1337/1.1 (UNIX)\r\n"
			"Last-Modified: "+fileDate2+"\r\n"
			"Content-Length: " + _ultoa(size,intToStrBuff,10) +"\r\n"+
			"Content-Type: " +mime+"\r\n"+
			"\r\n";
}

int getStatus(string filename, string date)
{
	ifstream iFile(filename);
	string date2 = fileDate(filename);
	if(date.compare(date2) == 0 && iFile)
	{
		return 304;
	}
	else if(iFile)
		return 200;
	else
		return 404;
}

configT loadCfg(){
	configT newConfig;
	fstream file;
	int size = 0;
	string line;
	string config = "";
	file.open("cfg/config.cfg", ios::in);
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
							newConfig.httpPort.c_str(),
							newConfig.telnetPort.c_str(),
							newConfig.wwwPath.c_str(),
							newConfig.password.c_str(),
							newConfig.logPath.c_str());

	file.close();
	return newConfig;
}
unsigned __stdcall httpMainThread(void *pArg){
	if(on == true){
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
		HANDLE hThread;
		unsigned threadID;
		SOCKET s1;
		int x = 0;
		sockInfoT *sockInfo;
		sockInfo =(sockInfoT*) malloc(sizeof(sockInfoT));
		while(true){
			s1 = accept(s,&clientAddr,&clientAddrLen);
			if(on){
				if(s1 != INVALID_SOCKET) {
					
					char *hostName = (char* ) malloc(100);
					char portName[100];
					int ok = getnameinfo(&clientAddr,clientAddrLen,hostName,100,portName,100,NI_NUMERICSERV );
					sockInfo->s = s1;
					sockInfo->hostname = hostName;
					if(ok == 0) {
						//printf("Accept incoming from: %s at port %s\n",hostName,portName);
					}
					hThread = (HANDLE) _beginthreadex(NULL, 10, &httpThread, (void *) sockInfo, 0, &threadID);	
				}else {
					int err = WSAGetLastError();
					printf("%d\n",err);
				}
			}
			else{
				closesocket(s1);
			}
		}
		free(sockInfo);
		closesocket(s);
		WSACleanup();
		
	}
	_endthreadex(0);
	return 0;
}
unsigned __stdcall httpThread(void *pArg){
	threads++;
	sockInfoT *sock = (sockInfoT*) pArg;
	int ok;
	printf("Threads: %d\n", threads);
	SOCKET s1 = sock->s;
	// Skriv ut meddelandet fr�n klienten
	int iResult;
	char *inputHTTP = (char*) malloc(1024);
	iResult = recv(s1, inputHTTP, 1024, 0);
	//fwrite(inputHTTP,1,iResult,stdout);
	//fflush(stdout);
	char cmdHTTP[80];
	char filenameHTTP[80];
	char protocolHTTP[80];
	ok = sscanf(inputHTTP,"%s /%s %s",cmdHTTP,filenameHTTP,protocolHTTP);
	// Skicka tillbaka ett svar till klienten
	string filetype = getFiletype(filenameHTTP);
	ULONG size = 0;
	ULONG len = 0;
	// Skriv ut meddelandet fr�n klienten
	char *buffer;
	char *checkIMS = strstr(inputHTTP,"If-Modified-Since: ");
	string date = "";
	if(checkIMS != NULL)
	{
		int i = 19;
		
		while(checkIMS[i] != '\r')
		{
			date.append(1,checkIMS[i]);
			i++;
		}
	}
	int status = getStatus(filenameHTTP, date);
	buffer = loadBin(filenameHTTP,size, status);
	len = send(s1,buffer,size, 0);
			
	printToLog(sock->hostname, cmdHTTP, filenameHTTP, protocolHTTP, status, size);
	//free(buffer);
	// St�ng sockets
	closesocket(s1);
	threads--;
	_endthreadex(0);
	return 0;
}
unsigned __stdcall telnetThread(void *pArg){
	threads++;
		WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );

	// Skapa en socket 
	SOCKET s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	// Ta reda p� addressen till localhost:8081
	struct addrinfo *info;
    int ok = getaddrinfo("localhost","8081",NULL,&info);
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
	while(true)
	{
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

			// L�sa av 
			string cmd;
			string arg;
			char line[100];
			char *temp = (char *)malloc(1);

			int i = 0;
			send(s1,"Welcome, enter password to log in. press escape to exit\n\r",57, 1);
			while(true){
				while(*temp != '\n'){
					iResult = recv(s1, temp, 1, 0);
					if(*temp == 27 || iResult == 0) //escape st�nger telnetklienten och �terg�r till att lyssna p� s1
						break;
					line[i] = *temp;
				i++;
				}
				if(*temp == 27 || iResult == 0) //escape st�nger telnetklienten och �terg�r till att lyssna p� s1
						break;
				*temp = ' ';
				int j = sscanf(line, "%s", cmd.c_str());
				// l�gger till ett \0 p� slutet =)
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
	
					infile = fopen("loggfile.txt","r");
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

			// St�ng sockets
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
	_endthreadex(0);
	threads--;
	return 0;
}

string getDate(void)
{
	time_t rawtime;
	struct tm * timeinfo;
	char curDate[80];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (curDate, 80, "%a, %d %b %Y %X",timeinfo);
	string strCurDate = curDate;
	return strCurDate;
}

void printToLog(string hostname, string cmdHTTP, string filenameHTTP, string protocolHTTP, int status, ULONG fileSize) //
{
	FILE *outfile;
	string date = getDate();

	outfile = fopen("loggfile.txt","a");
	fprintf(outfile, "%s [%s GMT] \"%s %s %s\" %d %d\n", hostname.c_str(), date.c_str(), cmdHTTP.c_str(), filenameHTTP.c_str(), protocolHTTP.c_str(), status, fileSize);
	
	fclose(outfile);
}

string fileDate(string filename)
{
	char timeStr[ 100 ] = "";
	struct stat buf;
	
	if (!stat(filename.c_str(), &buf))//l�gg till hela s�kv�gen
	{
		strftime(timeStr, 100, "%a, %d %b %Y %H:%M:%S GMT", localtime( &buf.st_mtime));
	}
	string fileDate = timeStr;
	return fileDate;
}

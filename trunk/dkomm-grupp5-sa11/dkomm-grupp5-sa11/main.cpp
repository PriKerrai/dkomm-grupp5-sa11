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
	char *wwwPath;
	string password;
	char *logPath;
} configT;



void loadCfg();
string getMime(string filename);
string createHeader(string filename, ULONG size, int status);
char *loadBin(string filename, ULONG &size, int status);
char *mergeVector(char *vector1, ULONG size1, char *vector2, ULONG size2);
string getFiletype(string filename);
unsigned __stdcall httpThread(void *pArg);
unsigned __stdcall httpMainThread(void *pArg);
void printToLog(string hostname, string cmdHTTP, string filenameHTTP, string protocolHTTP, int status, ULONG fileSize);
string getDate(void);
string fileDate(string filename);
int getStatus(string filename, string date);
unsigned __stdcall telnetThread(void *pArg);
void initializeConfig();
string getFilePath(string fileName,int pathType);

char hostName[100];
int threads = 0;
bool on = true;
configT config;
int _tmain(int argc, _TCHAR* argv[]){
	initializeConfig();
	loadCfg();
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
	else return "application/octet-stream";
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
	string file = getFilePath(filename,0);
	
		
	binFile.open(file,ios::in|ios::binary|ios::ate);
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
	string fileDate2 = fileDate(getFilePath(filename,0));
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
	string file = getFilePath(filename,0);
	ifstream iFile(file);
	string date2 = fileDate(file);
	int x = date.compare(date2) ;
	if(date.compare(date2) == 0 && iFile)
	{
		return 304;
	}
	else if(iFile)
		return 200;
	else
		return 404;
}

void loadCfg(){
	//configT newConfig;
	fstream file;
	int size = 0;
	string line;
	string configContent = "";
	file.open("cfg/config.cfg", ios::in);
	while(file.good()){
		getline(file,line);
		size += line.length();
		configContent.append(line);
		configContent.append("\n");
	}
	int ok = sscanf(configContent._Myptr(),
							"httpPort: %s\n"
							"telnetPort: %s\n"
							"wwwPath: %s\n"
							"password: %s\n"
							"logPath: %s",	
							config.httpPort.c_str(),
							config.telnetPort.c_str(),
							config.wwwPath,
							config.password.c_str(),
							config.logPath);

	file.close();
	//return newConfig;
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

		// Ta reda på addressen till localhost:4567
		struct addrinfo *info;
		int ok = getaddrinfo("localhost",config.httpPort.c_str(),NULL,&info);
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
		HANDLE hThread;
		unsigned threadID;
		SOCKET s1;
		int x = 0;
		
//		sockInfo = (sockInfoT*) malloc(sizeof(sockInfoT));
//		sockInfo->hostname = (char *) malloc(100);
		while(true){
			s1 = accept(s,&clientAddr,&clientAddrLen);
			if(on){
				if(s1 != INVALID_SOCKET) {
					
					
					char portName[100];
					int ok = getnameinfo(&clientAddr,clientAddrLen,hostName,100,portName,100,NI_NUMERICSERV );
					//sockInfo->s = s1;
					//sockInfo->hostname = hostName;
					if(ok == 0) {
						//printf("Accept incoming from: %s at port %s\n",hostName,portName);
					}
					hThread = (HANDLE) _beginthreadex(NULL, 10, &httpThread, (void*) s1, 0, &threadID);	
				}else {
					int err = WSAGetLastError();
					printf("%d\n",err);
				}
			}
			else{
				closesocket(s1);
			}
		}
//		free(sockInfo);
		closesocket(s);
		WSACleanup();
		
	}
	_endthreadex(0);
	return 0;
}
unsigned __stdcall httpThread(void *pArg){
	threads++;
	int ok;
	printf("Threads: %d\n", threads);
	SOCKET s1 = (SOCKET) pArg;
	// Skriv ut meddelandet från klienten
	int iResult;
	char *inputHTTP = (char*) malloc(2048);
	iResult = recv(s1, inputHTTP, 2048, 0);
	fwrite(inputHTTP,1,iResult,stdout);
	fflush(stdout);
	char cmdHTTP[512];
	char *filenameHTTP =  (char*)malloc(512);
	for (int j = 0; j < 512;j++)
		filenameHTTP[j] = '\0';
	char protocolHTTP[512];
	ok = sscanf(inputHTTP,"%s %s %s",cmdHTTP,filenameHTTP,protocolHTTP);
	// Skicka tillbaka ett svar till klienten
	if(filenameHTTP[1] == '\0')
		filenameHTTP = "ho.html";
	else {
		ok = sscanf(inputHTTP,"%s /%s %s",cmdHTTP,filenameHTTP,protocolHTTP);
	}
	string filetype = getFiletype(filenameHTTP);
	ULONG size = 0;
	ULONG len = 0;
	// Skriv ut meddelandet från klienten
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
			
	printToLog(hostName, cmdHTTP, filenameHTTP, protocolHTTP, status, size);
	free(filenameHTTP);
	free(buffer);
	// Stäng sockets
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

	// Ta reda på addressen till localhost:8081
	struct addrinfo *info;
	int ok = getaddrinfo("localhost",config.telnetPort._Myptr(),NULL,&info);
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

			// Läsa av 
			string cmd;
			string arg;
			char *line;
			char *temp = (char *)malloc(1);

			int i = 0;
			send(s1,"Welcome, enter password to log in. press escape to exit\n\r",57, 1);
			while(true){
				line = (char*) malloc(256);
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
				if ((cmd.compare("password") == 0) && !loggedIn){
					int pos = cmd.length();
					if(line[pos] != '\n'){ 
						int j = sscanf(line, "password %s",arg.c_str());
						arg = arg.c_str();

						if(arg.compare(config.password.c_str()) == 0&& arg[0] != '\0'){
							loggedIn = true;
							send(s1,"You are now logged in\n\r",23, 1);
						}
						else {
						loggedIn = false;
						send(s1,"Invalid password\n\r",18, 1);
						}
					}else send(s1,"Invalid password\n\r",18, 1);
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
	
					infile = fopen(getFilePath("log.txt",1).c_str(),"r");
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
				free(line);
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
	string file = getFilePath("log.txt", 1);
	outfile = fopen(file.c_str(),"a");
	fprintf(outfile, "%s [%s GMT] \"%s %s %s\" %d %d\n", hostname.c_str(), date.c_str(), cmdHTTP.c_str(), filenameHTTP.c_str(), protocolHTTP.c_str(), status, fileSize);
	
	fclose(outfile);
}

string fileDate(string filename)
{
	char timeStr[ 100 ] = "";
	struct stat buf;
	//string file = getFilePath(filename, 0);
	
	if (!stat(filename.c_str(), &buf))//lägg till hela sökvägen
	{
		strftime(timeStr, 100, "%a, %d %b %Y %H:%M:%S GMT", localtime( &buf.st_mtime));
	}
	string fileDate = timeStr;
	return fileDate;
}

void initializeConfig(){
	config.httpPort = "";
	config.telnetPort = "";
	config.wwwPath = (char*) malloc(50);
	config.password = "";
	config.logPath = (char*) malloc(50);
}

string getFilePath(string fileName, int pathType){
	char tempChar = ' ';
	string file = "";
	int i = 0;
	int strLen = fileName.length();
	if(pathType == 0){
		while(true){
			tempChar = config.wwwPath[i];
			if(tempChar == 92)
				file.append("\\");
			else if (tempChar != '\0')
				file = file + config.wwwPath[i];
			else {
				for(int j = 0; j < strLen; j++){
					file = file + fileName[j];
				}
				break;
			}
			i++;
		}
	}else if(pathType == 1){
		while(true){
			tempChar = config.logPath[i];
			if(tempChar == 92)
				file.append("\\");
			else if (tempChar != '\0')
				file = file + config.logPath[i];
			else {
				for(int j = 0; j < strLen; j++){
					file = file + fileName[j];
				}
				break;
			}
			i++;
		}
	}

	return file;
}
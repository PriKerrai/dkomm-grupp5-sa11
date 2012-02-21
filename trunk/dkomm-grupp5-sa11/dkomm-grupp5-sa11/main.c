#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
 
int _tmain(int argc, _TCHAR* argv[])
{      
        SOCKET s;
        struct addrinfo *info;
        int ok;
        char * message;
        int iResult;
        char buffer[512];
        WCHAR * error;
        // Initiera WinSock
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
        wVersionRequested = MAKEWORD( 2, 2 );
        err = WSAStartup( wVersionRequested, &wsaData );
 
        // Skapa en socket
        s = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
 
        // Ta reda på addressen till www.bt.se:80
       
    ok = getaddrinfo("www.bt.se","80",NULL,&info);
        error = gai_strerror(ok);
 
        // Anslut socketen till addressen
        ok = connect(s,info->ai_addr,info->ai_addrlen);
 
        // Skicka en request till webservern
        message = "GET / HTTP/1.1\r\nHOST: www.bt.se\r\n\r\n";
        ok = send(s,message,strlen(message),0);
        error = gai_strerror(ok);
 
        // Skriv ut svaret från webservern
       
        do {
                iResult = recv(s, buffer, 512, 0);
                if ( iResult > 0 ) {
                        fwrite(buffer,1,iResult,stdout);
                }
    } while( iResult > 0 );
        fflush(stdout);
 
        getchar();
 
        // Deinitiera WinSock
        WSACleanup();
        return 0;
}
/*
*/

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define PORT 1494

int main()
{
    int sd;
    sockaddr_in server = {};
    char buffer[100];
    
    if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Nu a putut fi creat socket-ul");
        return errno;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 0, 0);
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (sockaddr*)&server, sizeof(sockaddr))==-1){
        perror("Eroare la bind().\n");
        return errno;
    }

    bool quit = 0;
    while (!quit){
        sockaddr_in client = {};
        int msgLen;
        socklen_t addrLen = sizeof(sockaddr);


        msgLen = recvfrom(sd, buffer, sizeof(buffer), 0, (sockaddr*)&client, &addrLen);
        strcpy(buffer, "Buna buna");
        msgLen = sendto(sd, buffer, strlen(buffer), 0, (sockaddr*)&client, addrLen);
    }

    return 0;
}

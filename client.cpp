/*
*/

#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int sPort = 1494;

class Connection{
};

int main()
{
    int sd;
    sockaddr_in server;
    
    if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Eroare la socket()");
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("255.255.255.255");
    server.sin_port = htons(sPort);
    
    sendto(sd, "text", 5, 0, (sockaddr*)&server, sizeof(server));
    close(sd);
    
    return 0;
}

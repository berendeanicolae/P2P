/*
*/

#include <cstdio>
#include <cerrno>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1494

void msgProcess(){
}

enum MSG{
    PING,
    CONNECT,
    REQUEST,
    
};

int main()
{
    int sd;
    sockaddr_in server={};
    
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
        socklen_t sockSize;
        int len;
        char msg[100];
        sockaddr_in client={};
        
        if ( (len=recvfrom(sd, msg, 100, 0, (sockaddr *)&client, &sockSize)<=0) ){
            perror("Eroare la primire");
        }
        printf("%s", msg);
    }

    return 0;
}

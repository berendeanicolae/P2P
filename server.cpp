/*
*/

#include <sys/socket.h>
#include <sys/types.h>

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
    sockaddr_in server = {};
    
    if ( (sd=socket(AF_INET, SOCK_DGRAM, 0) == -1){
        perror("Nu a putut fi creat socket-ul");
        return errno;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 0, 0);
    
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = htonl(INADDR_ANY);
    if (bind(sd, (sockaddr*)&server, sizeof(sockaddr))==-1){
        perror("Eroare la bind().\n");
        return errno;
    }

    bool quit = 0;
    while (!quit){
        int len, sockSize;
        char msg[100];
        if ( (len=recvfrom(sd, msg, 100, 0, (sockaddr *)&client, &sockSize)<=0 ){
            perror("Eroare la primire");
        }
        printf("%s", msg);
    }

    return 0;
}

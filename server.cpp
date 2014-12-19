/*
*/

#include <cstdio>
#include <cerrno>
#include <pthread>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1494

/*
	Creaza un socket si ataseaza socket-ul noului socket
*/
bool startServer(int &sd, sockaddr_in &server){
	// creez socket
	if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        perror("Nu a putut fi creat socket-ul");
		return 1;
	}
	
	// setam optiunea SO_REUSEADDR
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 0, 0);
	
	// initalizam structura server si atasam socket-ul
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (sockaddr*)&server, sizeof(sockaddr))==-1){
        perror("Eroare la bind().\n");
        return 1;
    }
	
	return 0;
}

void* address(void *ptr, int value){
	return ((char*)ptr)+value;
}

void* msgProcess(void* arg){
	if (arg){
        sockaddr_in client;
		socklen_t sockSize;
		
		memcpy(&sockSize, address(arg, 0), sizeof(sockSize));
		memcpy(&client, address(arg, sizeof(sockSize)), sockSize);
		//extragem adresa de unde am primit mesajul
		//extragem mesajul
		delete [] arg;
	}
	//marcam threadul ca detached (nu mai e necasar join)
	pthread_detach(pthread_self());
	pthread_exit(0);
}

int main()
{
    int sd;
    sockaddr_in server={};
    
	startServer(sd, server);

    bool quit = 0;
    while (!quit){
        int len;
		char *buffer = new char[512];
        char msg[100];
        
        if ( (len=recvfrom(sd, msg, 100, 0, (sockaddr *)address(buffer, 0), &sockSize)<=0) ){
            perror("Eroare la primire");
			continue;
        }
		memcpy(
		pthread_t tid = pthread_create(&tid, 0, msgProcess, buffer);
    }

    return 0;
}

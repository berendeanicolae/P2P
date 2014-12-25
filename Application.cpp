#include "Application.h"
#define PORT 39085
#define IP "85.122.23.145"

Application::Application(){
	if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("Eroare la creare socket");
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
    server.sin_port = htons(PORT);
}

void Application::run(){
    close(sd);
}
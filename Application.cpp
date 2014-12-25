#include "Application.h"
#include <algorithm>
#define PORT 39085
#define IP "85.122.23.145"

string intToString(int value){
    string str;
    
    while (value){
        str += value%10+'0';
        value /= 10;
    }
    for (int i=0; i<=str.size(); ++i){
        swap(str[i], str[str.size()-i-1]);
    }
    return str;
}

Application::Application(){
	if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("Eroare la creare socket");
	}
    
    string ipPort;
    ipPort += IP;
    ipPort += intToString(PORT);
    commands.push_back(make_pair(P2P_connect, ipPort));
    
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
    server.sin_port = htons(PORT);
}

Application::~Application(){
    close(sd);
}

void Application::process(){
    for (unsigned i=0; i<commands.size(); ++i){
    }
}

void Application::run(){
    state->listen(commands);
}

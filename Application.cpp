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
    for (unsigned i=0; i<=str.size(); ++i){
        swap(str[i], str[str.size()-i-1]);
    }
    return str;
}

Application::Application(): quit(0), state(0){
    //create socket
	if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("Eroare la creare socket");
	}

    //initialize server address
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
    server.sin_port = htons(PORT);

    //initial connect request
    connected = 0;
    connectTimeout = 3;
    string ipPort = string(IP)+" "+intToString(PORT);
    requests.push_back(make_pair(P2P_connectAsPeer, ipPort));

    //process initial connect request
    state = new Client();
    process();
}

Application::~Application(){
    if (state){
        delete state;
    }
    close(sd);
}

void Application::process(){
    for (unsigned i=0; i<requests.size(); ++i){
        switch (requests[i].first){
            case P2P_connectAsPeer:
                break;
            case P2P_connectAsServer:
                break;
            case P2P_connectedOK:
                break;
            default:
                //wrong code
                break;
        }
    }
    requests.clear();

    if (!connected){
        if (connectTimeout>0){
            --connectTimeout;
            return;
        }
        else{
            //daca a trecut connectionTimeout
            //Nu exista server in retea, deci trec in mod server

            // optiunea de reutilizare a adresei pentru socket
            int opt;

            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            server.sin_port = htons (PORT);

            opt = 1;
            // setez optiunea de a reutiliza portul
            setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
            // atasez socketul
            if (bind(sd, (sockaddr*)&server, sizeof(server))==-1){
                perror("[server]Eroare la bind()");
                quit = 1;
            }
            // schimb state din client in server
            delete state;
            state = new Server;
        }
    }
}

void Application::run(){
    while (!quit){
        if (state)
            state->listen(requests);
        process();
    }
}

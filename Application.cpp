#include "Application.h"
#include <pwd.h>
#include <algorithm>
#include <cstring>
#define PORT 39085
//#define IP "85.122.23.145"
#define IP "127.0.0.1"

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
    struct passwd *pw;
    if ( !(pw=getpwuid(getuid())) ){
        perror("Eroare la getpwuid(getuid())");
        return;
    }
    //inirtializam directorul shared
    shared = pw->pw_dir;
    root = new Root(shared);

    socklen_t sz=sizeof(server);
    sds[nfds] = 2;
    //create socket UDP
	if ( (sds[udpsd]=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("Eroare la creare socket UDP");
	}
	sds[nfds] = max(sds[nfds], sds[udpsd]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(0);
	bind(sds[udpsd], (sockaddr*)&server, sizeof(server));
	getsockname(sds[udpsd], (sockaddr*)&server, &sz);
	pts[udpport] = server.sin_port;

    //create socket TCP
	if ( (sds[tcpsd]=socket(AF_INET, SOCK_STREAM, 0)) == -1){
	    perror("Eroare la creare socket TCP");
	}
	sds[nfds] = max(sds[nfds], sds[tcpsd]);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(0);
	bind(sds[tcpsd], (sockaddr*)&server, sizeof(server));
	listen(sds[tcpsd], 5);
	getsockname(sds[tcpsd], (sockaddr*)&server, &sz);
	pts[tcpport] = server.sin_port;

    //initial connect request
    connected = 0;
    ticksSinceOffline = getTicks();
    connectTimeout = 3;
    string ipPort = string(IP)+" "+intToString(PORT);
    requests.push_back(make_pair(P2P_connectAsPeer, ipPort));

    //process initial connect request
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(IP);
    server.sin_port = htons(PORT);
    state = new Client(sds, pts);
    process();
}

Application::~Application(){
    if (state){
        delete state;
    }
    close(sds[udpsd]);
    close(sds[tcpsd]);
}

void Application::process(){
    socklen_t sock_size = sizeof(server);
    char msg[100]={};

    for (unsigned i=0; i<requests.size(); ++i){
        action type=requests[i].first;
        memset(msg, 0, sizeof(msg));

        switch (type){
            case P2P_connectAsPeer:
            case P2P_connectAsServer:
                memcpy(msg, &type, sizeof(type));
                memcpy(msg+sizeof(type), requests[i].second.c_str(), requests[i].second.size());
                sendto(sds[udpsd], msg, sizeof(msg), 0, (sockaddr*)&server, sock_size);
                break;
            case P2P_connectedOK:
                connected = 1;
                printf("[app] Aplicatia s-a conectat cu succes\n");
                break;
            case P2P_quit:
                quit = 1;
                break;
            default:
                //wrong code
                break;
        }
    }
    requests.clear();
}

void Application::checkIfConnected(){
    if (!connected){
        if (getTicks()-ticksSinceOffline > connectTimeout){
            //daca a trecut connectionTimeout
            //Nu exista server in retea, deci trec in mod server

            // optiunea de reutilizare a adresei pentru socket
            int opt;

            close(sds[udpsd]); //creem un nou socket si il atasam adresei dorite
            if ( (sds[udpsd]=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
                perror("Eroare la creare socket");
            }

            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            server.sin_port = htons (PORT);

            opt = 1;
            // setez optiunea de a reutiliza portul
            setsockopt(sds[udpsd], SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
            // atasez socketul
            if (bind(sds[udpsd], (sockaddr*)&server, sizeof(sockaddr))==-1){
                perror("[server]Eroare la bind()");
                quit = 1;
                return;
            }
            // schimb state din client in server
            delete state;
            state = new Server(sds, pts);
            connected = 1;
            printf("Nu a fost gasit server de bootstrap.\n");
        }
    }
}

void Application::run(){
    while (!quit){
        if (state)
            state->listen(requests);
        process();
        checkIfConnected();
        static int startTime=getTicks();
        static int x =-1;
        if (getTicksSince(startTime)!=x){
            x = getTicksSince(startTime);
            //printf("Timer: %u seconds\n", x);
        }
    }
}


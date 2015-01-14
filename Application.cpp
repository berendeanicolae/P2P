#include "Application.h"
#include <pwd.h>
#include <algorithm>
#include <cstring>
#define PORT 39085
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

Application::Application(): quit(0), state(0), root(0){
    struct passwd *pw;
    socklen_t size=sizeof(server);
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
	if (bind(sds[udpsd], (sockaddr*)&server, sizeof(server))<0){
	    perror("Eroare la UDP bind()");
	}
	getsockname(sds[udpsd], (sockaddr*)&server, &size);
	printf("UDP port: %d\n", htons(server.sin_port));
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
	if (bind(sds[tcpsd], (sockaddr*)&server, sizeof(server))<0){
	    perror("Eroare la TCP bind()");
	}
	getsockname(sds[tcpsd], (sockaddr*)&server, &size);
	printf("TCP port: %d\n", htons(server.sin_port));
	listen(sds[tcpsd], 5);
	getsockname(sds[tcpsd], (sockaddr*)&server, &sz);
	pts[tcpport] = server.sin_port;

    //initial connect request
    connected = 0;
    ticksSinceOffline = getTicks();
    connectTimeout = 3;
    Message msg;
    msg.push_back(MSG_connectAsPeer);
    msg.push_back(strlen(IP), IP);
    msg.push_back(strlen(intToString(PORT).c_str()), intToString(PORT).c_str());
    requests.push_back(msg);

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
    if (root){
        delete root;
    }
    close(sds[udpsd]);
    close(sds[tcpsd]);
}

void Application::process(){
    socklen_t sock_size = sizeof(server);

    for (unsigned i=0; i<requests.size(); ++i){
        MSG msgType;
        Message msg;
        FileDir *file=0;
        int ip;
        unsigned short port;
        char *uuid, *exp;

        requests[i].pop_front(msgType);

        switch (msgType){
            case MSG_connectAsPeer:
            case MSG_connectAsServer:
                msg.clear();
                msg.push_back(msgType);
                sendto(sds[udpsd], msg.getMessage(), msg.getSize(), 0, (sockaddr*)&server, sock_size);
                break;
            case MSG_connectedOK:
                connected = 1;
                printf("[app] Aplicatia s-a conectat cu succes\n");
                break;
            case MSG_quit:
                quit = 1;
                break;
            case MSG_request:
                break;
            case MSG_search:
                requests[i].pop_front(&uuid);
                requests[i].pop_front(&ip);
                requests[i].pop_front(&port);
                requests[i].pop_front(&exp);
                printf("[search] Search %s %s from %d\n", exp, uuid, ntohs(port));
                file = root->find(exp);
                if (file){
                    int sd = socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in server={};
                    server.sin_family = AF_INET;
                    server.sin_addr.s_addr = ip;
                    server.sin_port = port;
                    if (connect(sd, (sockaddr*)&server, sizeof(server)) < 0){
                        perror("Eroare la connect()");
                    }
                    ///creem un nou descriptor pe care sa il conectam cu destinatia
                    ///creem o structura care sa memoreze informatiile despre download
                    printf("%s found\n", exp);
                }
                else{
                    printf("%s not found\n", exp);
                }
                delete[] exp;
                delete[] uuid;
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


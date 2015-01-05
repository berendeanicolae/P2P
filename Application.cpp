#include "Application.h"
#include "sha1.h"
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
    state = new Client(sd);
    process();
}

Application::~Application(){
    if (state){
        delete state;
    }
    close(sd);
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
                sendto(sd, msg, sizeof(msg), 0, (sockaddr*)&server, sock_size);
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
        if (connectTimeout>0){
            --connectTimeout;
            return;
        }
        else{
            //daca a trecut connectionTimeout
            //Nu exista server in retea, deci trec in mod server

            // optiunea de reutilizare a adresei pentru socket
            int opt;

            close(sd); //creem un nou socket si il atasam adresei dorite
            if ( (sd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
                perror("Eroare la creare socket");
            }

            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            server.sin_port = htons (PORT);

            opt = 1;
            // setez optiunea de a reutiliza portul
            setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
            // atasez socketul
            if (bind(sd, (sockaddr*)&server, sizeof(sockaddr))==-1){
                perror("[server]Eroare la bind()");
                quit = 1;
                return;
            }
            // schimb state din client in server
            delete state;
            state = new Server(sd);
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


long Application::getIP(){
    //must use some api to find the ip
    long ip = 0;
    return ip;
}

char intToHex(char digit){
    if (digit<10)
        return '0'+digit;
    if (digit<16)
        return 'a'+digit-10;
    return -1;
}

string Application::getUUID(){
    static unsigned char in[10], out[20];
    long ip=getIP(), timestamp=getTicks();
    short port=ntohs(server.sin_port), offset=0;
    string result;

    memset(in, 0, sizeof(in));

    memcpy(in+offset, &ip, sizeof(ip));
    offset += sizeof(ip);
    memcpy(in+offset, &port, sizeof(port));
    offset += sizeof(port);
    memcpy(in+offset, &timestamp, sizeof(timestamp));
    offset += sizeof(timestamp);

    sha1::calc(in, sizeof(in), out);
    for (unsigned i=0; i<sizeof(out); ++i){
        result += intToHex(out[i]&15);
        result += intToHex(out[i]&15<<4);
    }
    //combinatie intre IP, port, ticks (sha1)
    return result;
}

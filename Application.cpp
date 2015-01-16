#include "Application.h"
#include <pwd.h>
#include <algorithm>
#include <cstring>
#define PORT 39085
#define IP "127.0.0.1"

bool Application::connected = 0;
bool Application::quit = 0;

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

Application::Application(): state(0), root(0){
    struct passwd *pw;
    socklen_t size=sizeof(server);
    if ( !(pw=getpwuid(getuid())) ){
        perror("Eroare la getpwuid(getuid())");
        return;
    }
    //inirtializam directorul shared
    shared = pw->pw_dir;
    root = new Root(shared);

    nfds = 2;
    //create socket UDP
	if ( (udpsd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror("Eroare la creare socket UDP");
	}
	nfds = max(nfds, udpsd);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(0);
	if (bind(udpsd, (sockaddr*)&server, sizeof(server))<0){
	    perror("Eroare la UDP bind()");
	}
	getsockname(udpsd, (sockaddr*)&server, &size);
	printf("UDP port: %d\n", htons(server.sin_port));

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
    state = new Client(udpsd, nfds);
    process();
}

Application::~Application(){
    if (state){
        delete state;
    }
    if (root){
        delete root;
    }
    close(udpsd);
}

void* Application::download(void *p){
    fd_set readfds;
    int sd = *(int*)p;
    int lastResponse = getTicks();

    printf("Thread started\n");
    pthread_detach(pthread_self());
    while (!quit && getTicks()-lastResponse<10){
        FD_SET(sd, &readfds);
    }
    printf("Thread exited\n");
    pthread_exit(0);
    return 0;
}

void* Application::upload(void *){
    pthread_detach(pthread_self());
    printf("Thread upload\n");
    pthread_exit(0);
    return 0;
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
        int sd;
        pthread_t th;

        requests[i].pop_front(msgType);

        switch (msgType){
            case MSG_connectAsPeer:
            case MSG_connectAsServer:
                msg.clear();
                msg.push_back(msgType);
                sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&server, sock_size);
                break;
            case MSG_connectedOK:
                connected = 1;
                printf("[app] Aplicatia s-a conectat cu succes\n");
                break;
            case MSG_quit:
                quit = 1;
                break;
            case MSG_request:
                msg.pop_front(&sd);
                pthread_create(&th, 0, Application::download, &sd);
                //pornesc un dowload
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
                        continue;
                    }
                    Message *comm = new Message;
                    comm->push_back(sizeof(sd), &sd);
                    comm->push_back(sizeof(file), file);
                    delete comm; //to be removed after thread creadte
                    /*
                        pornim un thread care sa trimita date
                    //ne conectam la server si ii trimitem un mesaj MSG_have
                    int sd = socket(AF_INET, SOCK_STREAM, 0);
                    sockaddr_in server={};
                    server.sin_family = AF_INET;
                    server.sin_addr.s_addr = ip;
                    server.sin_port = port;
                    if (connect(sd, (sockaddr*)&server, sizeof(server)) < 0){
                        perror("Eroare la connect()");
                        continue;
                    }
                    msg.clear();
                    msg.push_back(MSG_have);
                    write(sd, msg.getPSize(), sizeof(msg.getPSize()));
                    write(sd, msg.getMessage(), msg.getSize());
                    ///pornim un thread de unde sa ascultam portul
                    printf("%s found\n", exp);
                    */
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

            close(udpsd); //creem un nou socket si il atasam adresei dorite
            if ( (udpsd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){
                perror("Eroare la creare socket");
            }

            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            server.sin_port = htons (PORT);

            opt = 1;
            // setez optiunea de a reutiliza portul
            setsockopt(udpsd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));
            // atasez socketul
            if (bind(udpsd, (sockaddr*)&server, sizeof(sockaddr))==-1){
                perror("[server]Eroare la bind()");
                quit = 1;
                return;
            }
            // schimb state din client in server
            delete state;
            state = new Server(udpsd, nfds);
            connected = 1;
            printf("Nu a fost gasit server de bootstrap.\n");
        }
    }
}

void Application::run(){
    while (!quit){
        if (state)
            state->listening(requests);
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


#include "Application.h"
#include <pwd.h>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#define PORT 39085
#define IP "127.0.0.1"

bool Application::connected = 0;
bool Application::quit = 0;
    string Application::shared("");

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

Application::Application(const char *shared_): state(0), root(0){
    //struct passwd *pw;
    socklen_t size=sizeof(server);
    /*if ( !(pw=getpwuid(getuid())) ){
        perror("Eroare la getpwuid(getuid())");
        return;
    }*/
    //inirtializam directorul shared
    //shared = pw->pw_dir;
    shared = shared_;
    root = new Root(shared.c_str());

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
    Message *arg=(Message*)p;
    FileDir *file=0;
    int tcpsd, lastResponse=getTicks(), nfds;
    vector<pair<int, int>> peers;
    vector<int> seeders;
    vector<FileDir*> files;
    char *strct=0;
    unsigned size=0, robin=0;
    vector<pair<bool, int>> downloaded;

    printf("Thread started\n");
    pthread_detach(pthread_self());

    arg->pop_front(&tcpsd);
    delete arg;
    nfds = tcpsd;

    while (!quit && getTicks()-lastResponse<10){
        fd_set readfds;
        timeval timeout={1, 0};

        FD_ZERO(&readfds);

        for (unsigned i=0; i<peers.size(); ++i){
            if (getTicks()-peers[i].second>10){
                swap(peers[i], peers.back());
                peers.pop_back();
            }
        }

        FD_SET(tcpsd, &readfds);
        for (auto it=peers.begin(); it!=peers.end(); ++it) {FD_SET(it->first, &readfds);}
        select(nfds+1, &readfds, 0, 0, &timeout);
        if (FD_ISSET(tcpsd, &readfds)){
            int sd = accept(tcpsd, 0, 0);
            lastResponse = getTicks();
            peers.push_back(make_pair(sd, getTicks()));
            nfds = max(nfds, sd);
            FD_CLR(tcpsd, &readfds);
            printf("accepted peer\n");
        }
        for (int d=3; d<=nfds; ++d){
            if (FD_ISSET(d, &readfds)){
                int size;

                if (read(d, &size, sizeof(size)) <= 0){
                    close(d);
                    continue;
                }
                lastResponse = getTicks();
                char *buffer = new char[size];
                read(d, buffer, size);
                Message msg(size, buffer);
                delete[] buffer;
                MSG msgType;
                msg.pop_front(msgType);
                switch (msgType){
                    case MSG_have:
                        seeders.push_back(d);
                        msg.clear();
                        msg.push_back(MSG_getstruct);
                        write(d, msg.getPSize(), sizeof(*msg.getPSize()));
                        write(d, msg.getMessage(), msg.getSize());
                        printf("[thread] struct requested\n");
                        break;
                    case MSG_struct:
                        if (!file){
                            msg.pop_front(&strct);
                            printf("%s\n", strct);
                            file = FileDir::createTree(strct);
                            file->create(shared);
                            file->getFiles(files);
                            delete[] strct;
                            strct = 0;
                        }
                        break;
                    case MSG_filesize:
                        msg.pop_front(&size);
                        if (!size){
                            printf("%s has size 0\n", files.back()->getPath().c_str());
                            files.pop_back(); //fisierul e gol
                        }
                    default:
                        printf("[thread] data received\n");
                        break;
                }
            }
        }
        if (files.size()){
            Message msg;

            if (size){
            }
            else{
                string name;
                msg.push_back(MSG_getfilesize);
                name = files.back()->getPath();
                msg.push_back(name.size(), name.c_str());
                printf("requesting size of %s\n", name.c_str());
                if (robin<seeders.size()){
                    write(seeders[robin], msg.getPSize(), sizeof(*msg.getPSize()));
                    write(seeders[robin], msg.getMessage(), msg.getSize());
                    robin++;
                    robin %= seeders.size();
                }
                else{
                    robin = 0;
                }
            }
        }
        else{
        }
    }

    if (file)
        delete file;
    printf("Thread exited\n");
    pthread_exit(0);
    return 0;
}

void* Application::upload(void *p){
    Message *arg=(Message*)p, msg;
    FILE *tin;
    int tcpsd, lastRequest=getTicks();
    FileDir *file;
    int size;
    string strct, path;
    char *name;
    struct stat stat_buf;


    pthread_detach(pthread_self());
    printf("Thread upload\n");

    arg->pop_front(&tcpsd);
    arg->pop_front(&file);
    arg->clear();

    arg->push_back(MSG_have);
    if (write(tcpsd, arg->getPSize(), sizeof(*arg->getPSize())) <= 0){
        perror("Eroare la send");
    }
    if (write(tcpsd, arg->getMessage(), arg->getSize()) <= 0){
        perror("Eroare la send");
    }
    delete arg;
    printf("sent data\n");
    while (!quit && getTicks()-lastRequest<10){
        fd_set readfds;
        timeval timeout={1, 0};

        FD_ZERO(&readfds);

        FD_SET(tcpsd, &readfds);
        select(tcpsd+1, &readfds, 0, 0, &timeout);
        if (FD_ISSET(tcpsd, &readfds)){
            if (read(tcpsd, &size, sizeof(size)) <= 0){
                break; //daca a fost inchis capatul remote, terminam threadul
            }
            char *buffer = new char[size];
            read(tcpsd, buffer, size);
            Message msg(size, buffer);
            delete[] buffer;
            MSG msgType;
            msg.pop_front(msgType);
            switch (msgType){
                default:
                    break;
                case MSG_getstruct:
                    //printf("[thread] struct send\n");
                    file->getStructure(strct);
                    msg.clear();
                    msg.push_back(MSG_struct);
                    msg.push_back(strct.size(), strct.c_str());
                    write(tcpsd, msg.getPSize(), sizeof(*msg.getPSize()));
                    write(tcpsd, msg.getMessage(), msg.getSize());
                    break;
                case MSG_getfilesize:
                    msg.pop_front(&name);
                    path = file->getPath();
                    while (path.back()!='/') path.pop_back();
                    path += name;
                    delete[] name;
                    printf("sending size of %s\n", path.c_str());
                    if (!stat(path.c_str(), &stat_buf)){
                        size = stat_buf.st_size;
                    }
                    msg.clear();
                    msg.push_back(MSG_filesize);
                    msg.push_back(sizeof(size), &size);
                    write(tcpsd, msg.getPSize(), sizeof(*msg.getPSize()));
                    write(tcpsd, msg.getMessage(), msg.getSize());
                    break;
            }
        }
    }

    printf("Thread exited\n");
    pthread_exit(0);
    return 0;
}

void Application::process(){
    socklen_t sock_size = sizeof(server);

    for (unsigned i=0; i<requests.size(); ++i){
        MSG msgType;
        Message msg, *comm;
        FileDir *file=0;
        int ip, sd;
        unsigned short port;
        char *uuid, *exp;
        pthread_t td;

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
                requests[i].pop_front(&sd);
                comm = new Message;
                comm->push_back(sizeof(sd), &sd);
                pthread_create(&td, 0, Application::download, comm);
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
                    comm = new Message;
                    comm->push_back(sizeof(sd), &sd);
                    comm->push_back(sizeof(&file), &file);
                    pthread_create(&td, 0, upload, comm);
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


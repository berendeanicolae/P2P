#include "Client.h"
#include "network.h"
#include <cstring>

Client::Client(int udpsd, int nfds, sockaddr_in *server): State(udpsd, nfds), lastPing(getTicks()){
    memcpy(&serverAddr, server, sizeof(*server));
}

void Client::ping() {printf("[client] client cannot ping");}

int Client::listening(vector<Message> &commands, int timeOut){
    fd_set readfds, writefds, errorfds;
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds);
    FD_SET(udpsd, &readfds);

    timeout.tv_sec = timeOut/1000;
    timeout.tv_usec = timeOut%1000;
    select(nfds+1, &readfds, &writefds, &errorfds, &timeout);

    cleanUUIDs();

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
        Message msg, comm;
        memset(msgBuffer, 0, sizeof(msgBuffer));
        if (read(0, msgBuffer, sizeof(msgBuffer))<=0){
            perror("[server] Eroare la citirea de la tastatura");
        }
        string input = msgBuffer;
		input.erase(input.end()-1, input.end()); //stergem caracterul linie noua
        if (input == "quit" || input == "exit"){
            msg.push_back(MSG_quit);
            commands.push_back(msg);
        }
        else if (!input.compare(0, strlen("search"), "search")){
            const char *uuid = getUUID();
            int sd = socket(AF_INET, SOCK_STREAM, 0); //socketul folosit pentru a comunica cu peers ce au fisierul
            sockaddr_in server={};
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(INADDR_ANY);
            server.sin_port = htons(0);
            bind(sd, (sockaddr*)&server, sizeof(server));
            listen(sd, 5);
            //sockaddr_in tcpserver = {};
            //socklen_t sz = sizeof(tcpserver);

            //calculam expresia regulata
            input.erase(input.begin(), input.begin()+strlen("search"));
            stringStrip(input);
            printf("[Search] %s %s\n", input.c_str(), uuid);
            uuids[uuid] = getTicks();
            comm.push_back(MSG_request);
            comm.push_back(sizeof(sd), &sd);
            commands.push_back(comm);
            //trimit la server mesajul
            int ip=getIP(sd);
            unsigned short port=getPort(sd);

            msg.clear();
            msg.push_back(MSG_search);
            msg.push_back(40, uuid);
            msg.push_back(sizeof(ip), &ip);
            msg.push_back(sizeof(port), &port);
            msg.push_back(input.size(), input.c_str());
            sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        }
        else{
        }
    }
    for (int d=3; d<=nfds; ++d){
        memset(msgBuffer, 0, sizeof(msgBuffer));

        if (FD_ISSET(d, &readfds)){// && d!=sd){
            MSG msgType;
            char ipString[40];
            int port, size;

            printf("[client] Am primit mesaj\n");
            socklen_t sock_size = sizeof(client);
            if ((size=recvfrom(d, msgBuffer, 100, 0, (sockaddr*)&client, &sock_size))<=0){
                printf("[client] S-a inchis");
                continue;
            }
            Message msg(size, msgBuffer);
            msg.pop_front(msgType);
            switch (msgType){
                default:
                    commands.push_back(msg);
                    break;
                case MSG_ping:
                    inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                    port = client.sin_port;
                    printf("[client] Ping from %s %d\n", ipString, port);
                    msg.clear();
                    msg.push_back(MSG_pong);
                    sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&client, sizeof(client));
                    lastPing = getTicks();
                    break;
            }
        }
    }
    if (getTicks()-lastPing > 20){
        Message msg;

        msg.push_back(MSG_disconnected);
        commands.push_back(msg);
        printf("[client] serverul e offline\n");
    }
    return 0;
}

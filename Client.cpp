#include "Client.h"
#include "network.h"
#include <cstring>

Client::Client(const int *sds, const unsigned short *pts): State(sds, pts) {}

void Client::ping() {printf("not implemented");}

int Client::listen(vector<Message> &commands, int timeOut){
    fd_set readfds, writefds, errorfds;
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds);
    FD_SET(sds[udpsd], &readfds);

    timeout.tv_sec = timeOut/1000;
    timeout.tv_usec = timeOut%1000;
    select(sds[nfds]+1, &readfds, &writefds, &errorfds, &timeout);

    cleanUUIDs();

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
        Message msg;
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
        else{
        }
    }
    for (int d=3; d<=sds[nfds]; ++d){
        memset(msgBuffer, 0, sizeof(msgBuffer));

        if (FD_ISSET(d, &errorfds)){// && d!=sd){
        }
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
                    memset(&msgBuffer, 0, sizeof(msgBuffer));
                    msgType = MSG_pong;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sds[udpsd], msgBuffer, sizeof(msgType), 0, (sockaddr*)&client, sizeof(client));
                    break;
                case MSG_searchNoIP:{
                    inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                    port = client.sin_port;
                    char *uuid=0, *exp=0;

                    msg.pop_front(&uuid);
                    msg.pop_front(&exp);
                    uuids[uuid] = getTicks();
                    printf("[client] Search %s uuid %s\n", exp, uuid);
                    msg.clear();
                    msg.push_back(MSG_search);
                    msg.push_back(40, uuid);
                    msg.push_back(strlen(ipString), ipString);
                    msg.push_back(strlen(exp), exp);
                    commands.push_back(msg);
                    delete[] uuid;
                    delete[] exp;
                    break;
                }
            }
        }
        if (FD_ISSET(d, &writefds)){// && d!=sd){
        }
    }
    return 0;
}

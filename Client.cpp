#include "Client.h"
#include "network.h"
#include <cstring>

Client::Client(sockaddr_in server_, int sd_): server(server_), sd(sd_){
    nfds = sd;
}

unsigned short Client::getPort(){
    return server.sin_port;
}

void Client::ping() {printf("not implemented");}

int Client::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set readfds, writefds, errorfds;
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds);
    FD_SET(sd, &readfds);

    timeout.tv_sec = timeOut/1000;
    timeout.tv_usec = timeOut%1000;
    select(nfds+1, &readfds, &writefds, &errorfds, &timeout);

    cleanUUIDs();

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
        memset(msgBuffer, 0, sizeof(msgBuffer));
        if (read(0, msgBuffer, sizeof(msgBuffer))<=0){
            perror("[server] Eroare la citirea de la tastatura");
        }
        string input = msgBuffer;
		input.erase(input.end()-1, input.end()); //stergem caracterul linie noua
        //input.pop_back(); //stergem caracterul linie noua
        if (input == "quit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else if (input == "exit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else{
        }
    }
    for (int d=3; d<=nfds; ++d){
        memset(msgBuffer, 0, sizeof(msgBuffer));

        if (FD_ISSET(d, &errorfds)){// && d!=sd){
        }
        if (FD_ISSET(d, &readfds)){// && d!=sd){
            action msgType;
            char ipString[40];
            int port;

            printf("[client] Am primit mesaj\n");
            socklen_t sock_size = sizeof(client);
            recvfrom(d, msgBuffer, 100, 0, (sockaddr*)&client, &sock_size);
            action *Paction=(action*)msgBuffer;
            msgType = *Paction;
            switch (msgType){
                default:
                    commands.push_back(make_pair(msgType, msgBuffer+4));
                    break;
                case P2P_ping:
                    inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                    port = client.sin_port;
                    printf("[client] Ping from %s %d\n", ipString, port);
                    memset(&msgBuffer, 0, sizeof(msgBuffer));
                    msgType = P2P_pong;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sd, msgBuffer, sizeof(msgType), 0, (sockaddr*)&client, sizeof(client));
                    break;
                case P2P_search:{
                    inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                    port = client.sin_port;
                    string uuid(msgBuffer+sizeof(msgType), msgBuffer+sizeof(msgType)+40);
                    uuids[uuid] = getTicks();
                    printf("[client] Search uuid %s\n", uuid.c_str());
                    break;
                }
            }
        }
        if (FD_ISSET(d, &writefds)){// && d!=sd){
        }
    }
    return 0;
}

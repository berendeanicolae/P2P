#include "Server.h"
#include "network.h"
#include <cstring>

Server::Server(int sd_): sd(sd_){
    nfds = sd;
}

int Server::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set readfds, writefds, errorfds; //multimile de descriptori de citire, scriere si eroare
    timeval timeout;
    char msg[100]={};
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_SET(0, &readfds); //includem tastatura in readfds
    FD_SET(sd, &readfds);

    timeout.tv_sec = timeOut/100;
    timeout.tv_usec = timeOut%100;
    select(nfds+1, &readfds, &writefds, &errorfds, &timeout);

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
        if (read(0, msg, sizeof(msg))<=0){
            perror("[server] Eroare la citirea de la tastatura");
        }
        string input = msg;
        input.pop_back(); //stergem caracterul linie noua
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
        if (FD_ISSET(d, &errorfds)){// && d!=sd){
        }
        if (FD_ISSET(d, &readfds)){// && d!=sd){
            int ip, port;

            socklen_t sock_size = sizeof(client);
            recvfrom(d, msg, 100, 0, (sockaddr*)&client, &sock_size);
            ip = client.sin_addr.s_addr;
            port = client.sin_port;
            printf("[server] S-a conectat un peer %d.%d.%d.%d %d\n", ip&255, (ip&255<<8)>>8, (ip&255<<16)>>ip, (ip&255<<24)>>24, port);

            action req=*(action*)msg, type;
            switch (req){
                case P2P_connectAsPeer:
                case P2P_connectAsServer:
                    printf("[server] Connection request\n");
                    sock_size = sizeof(client);
                    memset(msg, 0, sizeof(msg));
                    type = P2P_connectedOK;
                    memcpy(msg, &type, sizeof(type));
                    sendto(sd, msg, sizeof(msg), 0, (sockaddr*)&client, sock_size);
                    break;
                default:
                    printf("[server] Wrong option\n");
            }
        }
        if (FD_ISSET(d, &writefds)){// && d!=sd){
        }
    }
    return 0; //maybe return value is not needed
}

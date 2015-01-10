#include "Server.h"
#include "network.h"
#include <cstring>

Server::Server(sockaddr_in server_, int sd_): server(server_), sd(sd_), pingAfter(10), pingTimeout(20), maxPingTries(3), betweenPings(1){
    nfds = sd;
}

unsigned short Server::getPort(){
    return server.sin_port;
}

void Server::ping(){
    for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
        if (getTicks()-it->lastPong > pingAfter){ ///a trecut timp (pot da ping)
            if (it->tries<maxPingTries){ ///daca mai putem trimite ping-uri
                if (getTicks()-it->lastPing >= betweenPings){ ///a trecut destul timp de la ultimul ping
                    char ipString[40]={};
                    int port = it->address.sin_port;

                    memset(msgBuffer, 0, sizeof(msgBuffer));
                    action msgType = P2P_ping;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&it->address, sizeof(it->address));
                    inet_ntop(it->address.sin_family, &it->address.sin_addr.s_addr, ipString, sizeof(ipString));
                    printf("[server] Ping %s %d\n", ipString, port);
                    ++it->tries;
                    it->lastPing = getTicks();
                }
            }
            if (getTicks()-it->lastPong > pingTimeout){ ///nu a raspuns
                char ipString[40]={};
                int port = it->address.sin_port;

                inet_ntop(it->address.sin_family, &it->address.sin_addr.s_addr, ipString, sizeof(ipString));
                printf("[server] %s %d inaccesibil\n", ipString, port);
                it = peers.erase(it); ///trebuie sa verific daca e si in serverPeers (si sa-l sterg si de acolo daca apare)
                --it;
            }
        }
    }
}

int Server::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set readfds, writefds, errorfds; //multimile de descriptori de citire, scriere si eroare
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds); //includem tastatura in readfds
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
		input.erase(input.end()-1, input.end());
        //input.pop_back(); //stergem caracterul linie noua
        if (input == "quit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else if (input == "exit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else if (!input.compare(0, strlen("search"), "search")){
            const char *uuid = getUUID();
            printf("[Search] %s\n", uuid);
            uuids[uuid] = getTicks();
            //trimit la toti peeri mesajul
            for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                int offset = 0;
                action msgType = P2P_search;

                memset(msgBuffer, 0, sizeof(msgBuffer));
                memcpy(msgBuffer+offset, &msgType, sizeof(msgType));
                offset += sizeof(msgType);
                memcpy(msgBuffer+offset, uuid, strlen(uuid));
                offset += strlen(uuid);
                ///adaugam si expresia cautata
                ///punem si in commands

                sendto(sd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&it->address, sizeof(it->address));
            }
        }
        else{
        }
    }
    for (int d=3; d<=nfds; ++d){
        if (FD_ISSET(d, &errorfds)){// && d!=sd){
        }
        if (FD_ISSET(d, &readfds)){// && d!=sd){
            char ipString[40]={};
            int port;

            socklen_t sock_size = sizeof(client);
            recvfrom(d, msgBuffer, 100, 0, (sockaddr*)&client, &sock_size);
            inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
            port = client.sin_port;

            action *p = (action*)msgBuffer;
            action msgType=*p;
            switch (msgType){
                case P2P_connectAsServer:
                    printf("[server] Connection request P2P_connectAsServer\n");
                    serverPeers.push_back(client);
                case P2P_connectAsPeer:
                    printf("[server] Connection request\n");
                    printf("[server] S-a conectat un peer %s %d\n", ipString, port);
                    peers.push_back(client);
                    sock_size = sizeof(client);
                    memset(msgBuffer, 0, sizeof(msgBuffer));
                    msgType = P2P_connectedOK;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&client, sock_size);
                    ///inainte de push verificam sa nu fie in lista (nu e deja peer)
                    printf("[server] Numar total de conexiuni: %lu\n", peers.size());
                    break;
                case P2P_pong:
                    for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                        if (!memcmp(&it->address, &client, sizeof(it->address))){
                            ///client a raspuns la ping
                            printf("[server] Pong from %s %d\n", ipString, port);
                            it->lastPong = getTicks();
                            it->tries = 0;
                            break;
                        }
                    }
                    break;
                default:
                    printf("[server] Wrong option\n");
            }
        }
        if (FD_ISSET(d, &writefds)){// && d!=sd){
        }
    }
    ping();
    return 0; //maybe return value is not needed
}

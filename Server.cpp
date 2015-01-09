#include "Server.h"
#include "network.h"
#include <cstring>

Server::Server(int sd_): sd(sd_), pingAfter(10), pingTimeout(20), maxPingTries(3), betweenPings(1){
    nfds = sd;
}

void Server::ping(){
    for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
        if (getTicks()-it->lastPong > pingAfter){ ///a trecut timp (pot da ping)
            if (it->tries<maxPingTries){ ///daca mai putem trimite ping-uri
                if (getTicks()-it->lastPing >= betweenPings){ ///a trecut destul timp de la ultimul ping
                    int ip = it->address.sin_addr.s_addr;
                    int port = it->address.sin_port;

                    memset(msgBuffer, 0, sizeof(msgBuffer));
                    action msgType = P2P_ping;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&it->address, sizeof(it->address));
                    printf("[server] Ping %d.%d.%d.%d %d\n", ip&255, (ip&255<<8)>>8, (ip&255<<16)>>ip, (ip&255<<24)>>24, port);
                    ++it->tries;
                    it->lastPing = getTicks();
                }
            }
            if (getTicks()-it->lastPong > pingTimeout){ ///nu a raspuns
                int ip = it->address.sin_addr.s_addr;
                int port = it->address.sin_port;
                printf("[server] %d.%d.%d.%d %d inaccesibil\n", ip&255, (ip&255<<8)>>8, (ip&255<<16)>>ip, (ip&255<<24)>>24, port);
                it = peers.erase(it);
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

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
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
        else if (input == "search"){
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
            recvfrom(d, msgBuffer, 100, 0, (sockaddr*)&client, &sock_size);
            ip = client.sin_addr.s_addr;
            port = client.sin_port;

            action req=*(action*)msgBuffer, type;
            switch (req){
                case P2P_connectAsPeer:
                case P2P_connectAsServer:
                    printf("[server] Connection request\n");
                    printf("[server] S-a conectat un peer %d.%d.%d.%d %d\n", ip&255, (ip&255<<8)>>8, (ip&255<<16)>>ip, (ip&255<<24)>>24, port);
                    sock_size = sizeof(client);
                    memset(msgBuffer, 0, sizeof(msgBuffer));
                    type = P2P_connectedOK;
                    memcpy(msgBuffer, &type, sizeof(type));
                    sendto(sd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&client, sock_size);
                    peers.push_back(client);
                    ///inainte de push verificam sa nu fie in lista (nu e deja peer)
                    printf("[server] Numar total de conexiuni: %lu\n", peers.size());
                    break;
                case P2P_pong:
                    for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                        if (!memcmp(&it->address, &client, sizeof(it->address))){
                            ///client a raspuns la ping
                            ip = it->address.sin_addr.s_addr;
                            port = it->address.sin_port;
                            printf("[server] Pong from %d.%d.%d.%d %d\n", ip&255, (ip&255<<8)>>8, (ip&255<<16)>>ip, (ip&255<<24)>>24, port);
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

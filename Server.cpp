#include "Server.h"
#include "network.h"
#include <cstring>

Server::Server(const int *sds, const unsigned short *pts): State(sds, pts), pingAfter(10), pingTimeout(20), maxPingTries(3), betweenPings(1){
}

void Server::ping(){
    for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
        if (getTicks()-it->lastPong > pingAfter){ ///a trecut timp (pot da ping)
            if (it->tries<maxPingTries){ ///daca mai putem trimite ping-uri
                if (getTicks()-it->lastPing >= betweenPings){ ///a trecut destul timp de la ultimul ping
                    char ipString[40]={};
                    int port = it->address.sin_port;

                    memset(msgBuffer, 0, sizeof(msgBuffer));
                    MSG msgType = MSG_ping;
                    memcpy(msgBuffer, &msgType, sizeof(msgType));
                    sendto(sds[udpsd], msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&it->address, sizeof(it->address));
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

int Server::listen(vector<Message> &commands, int timeOut){
    fd_set readfds, writefds, errorfds; //multimile de descriptori de citire, scriere si eroare
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds); //includem tastatura in readfds
    FD_SET(sds[udpsd], &readfds);
    FD_SET(sds[tcpsd], &readfds);

    timeout.tv_sec = timeOut/1000;
    timeout.tv_usec = timeOut%1000;
    select(sds[nfds]+1, &readfds, &writefds, &errorfds, &timeout);

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
		input.erase(input.end()-1, input.end());
        //input.pop_back(); //stergem caracterul linie noua
        if (input == "quit" || input == "exit"){
            msg.push_back(MSG_quit);
            commands.push_back(msg);
        }
        else if (!input.compare(0, strlen("search"), "search")){
            const char *uuid = getUUID();
            sockaddr_in tcpserver = {};
            socklen_t sz = sizeof(tcpserver);

            //calculam expresia regulata
            input.erase(input.begin(), input.begin()+strlen("search"));
            stringStrip(input);
            printf("[Search] %s %s\n", input.c_str(), uuid);
            uuids[uuid] = getTicks();
            getsockname(sds[tcpsd], (sockaddr*)&tcpserver, &sz);///poate nu avem nevoie. vrem sa punem
            //trimit la toti peeri mesajul
            for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                int ip=getIP();
                msg.clear();
                msg.push_back(MSG_search);
                comm.push_back(MSG_request);
                msg.push_back(40, uuid);
                comm.push_back(40, uuid);
                msg.push_back(sizeof(ip), &ip);
                comm.push_back(sizeof(ip), &ip);
                msg.push_back(sizeof(pts[tcpport]), &pts[tcpport]);
                comm.push_back(sizeof(pts[tcpport]), &pts[tcpport]);
                msg.push_back(input.size(), input.c_str());
                comm.push_back(input.size(), input.c_str());
                commands.push_back(comm);

                sendto(sds[udpsd], msg.getMessage(), msg.getSize(), 0, (sockaddr*)&it->address, sizeof(it->address));
            }
        }
        else{
        }
    }
    for (int d=3; d<=sds[nfds]; ++d){
        if (FD_ISSET(d, &errorfds)){
        }
        if (FD_ISSET(d, &readfds)){
            if (d==sds[udpsd]){
                MSG msgType;
                char ipString[40];
                int port, size;

                socklen_t sock_size = sizeof(client);
                if ((size=recvfrom(d, msgBuffer, 100, 0, (sockaddr*)&client, &sock_size))<=0){
                    printf("Clientul s-a inchis");
                    continue;
                }

                Message msg(size, msgBuffer);
                msg.pop_front(msgType);
                switch (msgType){
                    case MSG_connectAsServer:
                        printf("[server] Connection request P2P_connectAsServer\n");
                        serverPeers.push_back(client);
                    case MSG_connectAsPeer:
                        inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                        port = client.sin_port;
                        printf("[server] Connection request\n");
                        printf("[server] S-a conectat un peer %s %d\n", ipString, port);
                        peers.push_back(client);
                        sock_size = sizeof(client);
                        msg.clear();
                        msg.push_back(MSG_connectedOK);
                        sendto(sds[udpsd], msg.getMessage(), msg.getSize(), 0, (sockaddr*)&client, sock_size);
                        ///inainte de push verificam sa nu fie in lista (nu e deja peer)
                        printf("[server] Numar total de conexiuni: %lu\n", peers.size());
                        break;
                    case MSG_pong:
                        inet_ntop(client.sin_family, &client.sin_addr.s_addr, ipString, sizeof(ipString));
                        port = client.sin_port;
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
                    case MSG_search:
                        break;
                    default:
                        printf("[server] Wrong option\n");
                }
            }
            else if (d==sds[tcpsd]){
                sockaddr_in client;
                socklen_t size;
                int sd;

                if ((sd=accept(sds[tcpsd], (sockaddr*)&client, &size)) < 0){
                    perror("Eroare la accept()");
                    continue;
                }
            }
            else{
            }
        }
        if (FD_ISSET(d, &writefds)){
        }
    }
    ping();
    return 0; //maybe return value is not needed
}

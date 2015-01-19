#include "Server.h"
#include "network.h"
#include <cstring>

Server::Server(int udpsd, int nfds): State(udpsd, nfds), pingAfter(10), pingTimeout(20), maxPingTries(3), betweenPings(1){
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
                    sendto(udpsd, msgBuffer, sizeof(msgBuffer), 0, (sockaddr*)&it->address, sizeof(it->address));
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

int Server::listening(vector<Message> &commands, int timeOut){
    fd_set readfds, writefds, errorfds; //multimile de descriptori de citire, scriere si eroare
    timeval timeout;
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_ZERO(&readfds);FD_ZERO(&writefds);FD_ZERO(&errorfds);
    FD_SET(0, &readfds); //includem tastatura in readfds
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
		input.erase(input.end()-1, input.end());
        //input.pop_back(); //stergem caracterul linie noua
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
            //trimit la toti peeri mesajul
            for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                int ip=getIP(sd);
                unsigned short port=getPort(sd);

                msg.clear();
                msg.push_back(MSG_search);
                msg.push_back(40, uuid);
                msg.push_back(sizeof(ip), &ip);
                msg.push_back(sizeof(port), &port);
                msg.push_back(input.size(), input.c_str());

                sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&it->address, sizeof(it->address));
            }
        }
        else{
        }
    }
    for (int d=3; d<=nfds; ++d){
        if (FD_ISSET(d, &errorfds)){
        }
        if (FD_ISSET(d, &readfds)){
            if (d==udpsd){
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
                        sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&client, sock_size);
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
                        commands.push_back(msg);
                        for (list<Peer>::iterator it=peers.begin(); it!=peers.end(); ++it){
                            sendto(udpsd, msg.getMessage(), msg.getSize(), 0, (sockaddr*)&it->address, sizeof(it->address));
                        }
                        break;
                    default:
                        printf("[server] Wrong option\n");
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

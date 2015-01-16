#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "State.h"
#include <vector>
#include <list>
#include <map>

class Server : public State{
    class Peer{
        public:
            sockaddr_in address;
            int lastPing, lastPong;
            int tries; ///number of pings sent
            Peer(sockaddr_in address_): address(address_), lastPing(getTicks()), lastPong(getTicks()), tries(0) {}
    };
    public:
        Server(int udpsd, int nfds);
        int listening(vector<Message> &commands, int timeOut);
    private:
        sockaddr_in server;
        void ping();
        list<Peer> peers, serverPeers; ///trebuie sa contina momentul ultimului raspuns pong si nr de incercari

        const int pingAfter, pingTimeout, maxPingTries, betweenPings;
};

#endif //SERVER_H_INCLUDED

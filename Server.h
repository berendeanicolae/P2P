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
        Server(int sd_=3);
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        int sd, nfds;
        void ping();
        list<Peer> peers; ///trebuie sa contina momentul ultimului raspuns pong si nr de incercari
        map<string, int> uuids; ///contine perechi (uuid, ticks)

        char msgBuffer[100];
        const int pingAfter, pingTimeout, maxPingTries, betweenPings;
};

#endif //SERVER_H_INCLUDED

#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        Client(sockaddr_in server_, int sd_=3);
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        sockaddr_in server;
        int sd, nfds;
        void ping();
        unsigned short getPort();
};

#endif

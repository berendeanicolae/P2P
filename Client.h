#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        Client(int udpsd, int nfds);
        int listening(vector<Message> &commands, int timeOut);
    private:
        int lastPing;
        sockaddr_in server;
        void ping();
};

#endif

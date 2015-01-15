#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        Client(const int *sds, const unsigned short *pts);
        int listening(vector<Message> &commands, int timeOut);
    private:
        sockaddr_in server;
        void ping();
};

#endif

#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        Client(const int *sds, const unsigned short *pts);
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        sockaddr_in server;
        void ping();
        unsigned short getPort();
};

#endif

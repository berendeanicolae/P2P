#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        Client(int sd_=3);
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        int sd, nfds;
        void ping();
};

#endif

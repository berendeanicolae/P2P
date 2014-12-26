#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include "State.h"

class Client : public State{
    public:
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        int sd, nfds;
};

#endif

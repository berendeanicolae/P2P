#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "State.h"

class Server : public State{
    public:
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        int sd, nfds;
};

#endif //SERVER_H_INCLUDED

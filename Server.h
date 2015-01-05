#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "State.h"
#include <vector>
#include <map>

class Server : public State{
    public:
        Server(int sd_=3);
        int listen(vector< pair<action, string> > &commands, int timeOut);
    private:
        int sd, nfds;
        vector<sockaddr_in> peers;
        map<string, int> uuids; ///contine perechi (uuid, ticks)
};

#endif //SERVER_H_INCLUDED

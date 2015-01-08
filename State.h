#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <vector>
#include <string>
#include "network.h"
using namespace std;

enum action{
    P2P_connectAsPeer,
    P2P_connectAsServer,
    P2P_connectedOK,
    P2P_quit,
    P2P_ping,
    P2P_pong,
    P2P_search
};

class State{
    public:
        State();
        virtual ~State();
        virtual int listen(vector< pair<action, string> > &commands, int timeOut=100)=0;
	protected:
		virtual void ping()=0;
};


#endif

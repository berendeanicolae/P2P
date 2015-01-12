#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <map>
#include <cstring>
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

enum sd{
    udpsd,
    tcpsd,
    nfds,
    sdmaxvalue
};
enum port{
    udpport,
    tcpport,
    portmaxvalue
};

class State{
    public:
        State(const int* sds_, const unsigned short *pts_);
        virtual ~State();
        virtual int listen(vector< pair<action, string> > &commands, int timeOut=100)=0;
	protected:
		virtual void ping()=0;
		const char* getUUID();
		char msgBuffer[100];
		map<string, int> uuids; ///perechi (hash, moment primire)
		void cleanUUIDs();
		int uuidLifetime;

		int sds[sdmaxvalue];
		unsigned short pts[portmaxvalue];
    private:
		int getIP();
};


#endif

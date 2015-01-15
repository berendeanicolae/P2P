#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <map>
#include <cstring>
#include <vector>
#include <string>
#include "network.h"
#include "Message.h"
using namespace std;

enum sd{ //sds
    udpsd,
    tcpsd,
    nfds,
    sdmaxvalue
};
enum port{ //ports
    udpport,
    tcpport,
    portmaxvalue
};

class State{
    public:
        State(const int* sds_, const unsigned short *pts_);
        virtual ~State();
        virtual int listening(vector<Message> &commands, int timeOut=100)=0;
	protected:
		virtual void ping()=0;
		const char* getUUID();
		virtual int getIP();
		char msgBuffer[100];
		map<string, int> uuids; ///perechi (hash, moment primire)
		void cleanUUIDs();
		int uuidLifetime;

		int sds[sdmaxvalue];
		unsigned short pts[portmaxvalue];
    protected: //aceste functii pot fi implementate in alt cpp (nu tin de state)
        void stringStrip(string& str);
};


#endif

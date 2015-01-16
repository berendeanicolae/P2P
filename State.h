#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <map>
#include <cstring>
#include <vector>
#include <string>
#include <map>
#include "network.h"
#include "Message.h"
using namespace std;

class State{
    public:
        State(int udpsd_, int nfds_);
        virtual ~State();
        virtual int listening(vector<Message> &commands, int timeOut=100)=0;
	protected:
	    int udpsd, nfds;
		virtual void ping()=0;
		const char* getUUID();
		int getIP(int sd);
		unsigned short getPort(int sd);
		char msgBuffer[100];
		map<string, int> uuids; ///perechi (hash, moment primire)
		void cleanUUIDs();
		int uuidLifetime;

    protected: //aceste functii pot fi implementate in alt cpp (nu tin de state)
        void stringStrip(string& str);
};


#endif

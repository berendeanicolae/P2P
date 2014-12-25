#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include <vector>
#include <string>
#include <sys/time.h>
using namespace std;

enum action{
    P2P_connect
};

class State{
    public:
        State();
        virtual ~State();
        virtual int listen(vector< pair<action, string> > &commands, int timeOut=60)=0;
};


#endif
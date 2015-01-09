#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include "network.h"
#include "State.h"
#include "Client.h"
#include "Server.h"
#include <vector>
#include <string>
using namespace std;


/**
    Manages the states.
*/

class Application{
    public:
        Application();
        ~Application();
        void run();
    private:
        //starea aplicatiei
        bool quit, connected;
        int connectTimeout;
    private:
        //
        int sd;
        sockaddr_in server;
    private:
        State *state;
        vector< pair<action, string> > requests;

        void process();
        void checkIfConnected();
};

#endif

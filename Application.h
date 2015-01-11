#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include "network.h"
#include "State.h"
#include "Client.h"
#include "Server.h"
#include "File.h"
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
        int connectTimeout, ticksSinceOffline;
    private:
        //
        int sd;
        sockaddr_in server;
    private:
        State *state;
        vector< pair<action, string> > requests;
        const char *shared;
        FileDir *root;

        void process();
        void checkIfConnected();
};

#endif

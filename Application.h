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
        int connectTimeout, ticksSinceOffline;
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


class FileDir{
    string name;
    FileDir *next;
};
class File: public FileDir{
};
class Dir: public FileDir{
};

#endif

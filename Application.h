#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

//networking and error checking
#include <cstdio>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
//
#include "State.h"
#include "Client.h"
#include <vector>
#include <string>
using namespace std;


/**
    Manages the states.
*/
//singleton

class Application{
    public:
        Application();
        ~Application();
        void run();

    private:
        void process();
        
        State *state;
        int sd; //socket descriptor
        sockaddr_in server;
        vector< pair<action, string> > commands;
};

#endif

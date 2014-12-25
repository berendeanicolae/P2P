#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include <cstdio>
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>

#include <vector>

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
        int sd; //socket descriptor
        sockaddr_in server;
        vector< pair<int, int> > actions;
};

#endif
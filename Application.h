#ifndef APPLICATION_H_INCLUDED
#define APPLICATION_H_INCLUDED

#include "network.h"
#include "State.h"
#include "Client.h"
#include "Server.h"
#include "File.h"
#include <vector>
#include <string>
#include <pthread.h>
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
        static bool quit, connected;
        int connectTimeout, ticksSinceOffline;
    private:
        sockaddr_in server;
    private:
        int udpsd, nfds;
        State *state;
        vector<Message> requests;
        const char *shared;
        FileDir *root;
        static void* download(void *);
        static void* upload(void *);

        void process();
        void checkIfConnected();
};


#endif

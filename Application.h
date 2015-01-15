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
/* am nevoie de:
    download: uuid, socket accept, lista de peers care au un fisier pentru fiecare download + timpul ultimului transfer
    upload: uuid, socket trimitere pentru fiecare upload + timpul ultimului transfer
*/
class Application{
    class Download{ //pt download-uri am nevoie de un map<uuid, Download> //fiecare download are un id unic
        public:
            int sd, lastResponse; //socketul folosit pentru a primi acest fisier, ultimul fisier primit
            vector<int> peers;//lista de socketuri cu cei ce au fisierul

    };
    class Upload{
        public:
            int sd, lastResponse;
    };

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
        int sds[sdmaxvalue]; //descriptors
        unsigned short pts[portmaxvalue]; //ports
        sockaddr_in server;
    private:
        State *state;
        vector<Message> requests;
        vector<pair<string, Download>> downloads;
        vector<pair<string, Upload>> uploads;
        const char *shared;
        FileDir *root;

        void process();
        void checkIfConnected();
};

#endif

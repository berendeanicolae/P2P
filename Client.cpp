#include "Client.h"
#include "network.h"
#include <cstring>

Client::Client(int sd_): sd(sd_){
    nfds = sd;
}

int Client::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set readfds, writefds, errorfds;
    timeval timeout;
    char msg[100]={};
    sockaddr_in client = {};

    //creem multimile de descriptori
    FD_SET(0, &readfds);
    FD_SET(sd, &readfds);

    timeout.tv_sec = timeOut/100;
    timeout.tv_usec = timeOut%100;
    select(nfds+1, &readfds, &writefds, &errorfds, &timeout);

    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    //verificam separat stdin, stdout, stderr
    if (FD_ISSET(0, &readfds)){
        if (read(0, msg, sizeof(msg))<=0){
            perror("[server] Eroare la citirea de la tastatura");
        }
        string input = msg;
		input.erase(input.end()-1, input.end()); //stergem caracterul linie noua
        //input.pop_back(); //stergem caracterul linie noua
        if (input == "quit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else if (input == "exit"){
            commands.push_back(make_pair(P2P_quit, ""));
        }
        else{
        }
    }
    for (int d=3; d<=nfds; ++d){
        memset(msg, 0, sizeof(msg));

        if (FD_ISSET(d, &errorfds)){// && d!=sd){
        }
        if (FD_ISSET(d, &readfds)){// && d!=sd){
            socklen_t sock_size = sizeof(client);
            recvfrom(d, msg, 100, 0, (sockaddr*)&client, &sock_size);
            commands.push_back(make_pair(*(action*)msg, msg+4));
            printf("[client] Am primit mesaj\n");
        }
        if (FD_ISSET(d, &writefds)){// && d!=sd){
        }
    }
    return 0;
}

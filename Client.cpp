#include "Client.h"

int Client::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set read, write, error;
    timeval timeout;
    
    //creem multimile de descriptori
    FD_SET(0, &read);
    
    timeout.tv_sec = timeOut/100;
    timeout.tv_usec = timeOut%100;
    select(nfds, &read, &write, &error, &timeout);
    
    /*
      Putem avea ori intrari de la tastatura (0), ori mesaje legate de retea (3), ori bucati de fisiere
    */
    for (int d=3; d<nfds; ++d){
        if (FD_ISSET(d, &error) && d!=sd){
        }
        if (FD_ISSET(d, &read) && d!=sd){
        }
        if (FD_ISSET(d, &write) && d!=sd){
        }
    }
}

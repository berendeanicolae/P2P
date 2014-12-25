#include "Client.h"

int Client::listen(vector< pair<action, string> > &commands, int timeOut){
    fd_set read, write, error;
    timeval timeout;
    
    //fd_set
    
    timeout.tv_sec = timeOut/100;
    timeout.tv_usec = timeOut%100;
    select(nfds, &read, &write, &error, &timeout);
    
    //check sd to see if there are peers requestiong files
    for (int d=3; d<nfds; ++d){
        if (FD_ISSET(d, &error) && d!=sd){
        }
        if (FD_ISSET(d, &read) && d!=sd){
        }
        if (FD_ISSET(d, &write) && d!=sd){
        }
    }
}
#include "State.h"
#include "sha1.h"
#include <cctype> //added for stringStrip

State::State(int udpsd_, int nfds_): udpsd(udpsd_), nfds(nfds_), uuidLifetime(30){
}
State::~State() {}

int State::getIP(int sd){
    sockaddr_in server;
    socklen_t size = sizeof(server);
    getsockname(sd, (sockaddr*)&server, &size);
    return server.sin_addr.s_addr;
}

unsigned short State::getPort(int sd){
    sockaddr_in server;
    socklen_t size = sizeof(server);
    getsockname(sd, (sockaddr*)&server, &size);
    return server.sin_port;
}

void State::stringStrip(string& str){
    string::iterator l, r;
    for (l=str.begin(); l!=str.end() && isspace(*l); ++l);
    for (r=str.end(); l!=r && isspace(*r); --r);
    str = string(l, r);
}

void State::cleanUUIDs(){
    /// Facem update la multimea de uuid-uri poate fi mutat intr-o functie (eg. update)
    for (map<string, int>::iterator it=uuids.begin(); it!=uuids.end();){ //nu e neaparat necesar
        if (getTicks()-it->second>uuidLifetime){
            printf("[server] removed uuid %s\n", it->first.c_str());
            uuids.erase(it++);
        }
        else{
            ++it;
        }
    }
}

const char *State::getUUID(){
    static unsigned char in[20], out[20]; ///2*long+short=18. Poate pe viitor trebuie marit
    long ip=getIP(udpsd), timestamp=getTicks();
    unsigned short port=getPort(udpsd), offset=0;
    static char result[41]={};

    memset(in, 0, sizeof(in));

    memcpy(in+offset, &ip, sizeof(ip));
    offset += sizeof(ip);
    memcpy(in+offset, &port, sizeof(port));
    offset += sizeof(port);
    memcpy(in+offset, &timestamp, sizeof(timestamp));
    offset += sizeof(timestamp);

    sha1::calc(in, offset, out);
    sha1::toHexString(out, result);
    //combinatie intre IP, port, ticks (sha1)
    return result;
}

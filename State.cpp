#include "State.h"
#include "sha1.h"
#include <cctype> //added for stringStrip

State::State(const int* sds_, const unsigned short *pts_): uuidLifetime(30){
    sds[udpsd] = sds_[udpsd];
    sds[tcpsd] = sds_[tcpsd];
    sds[nfds] = sds_[nfds];
    pts[udpport] = pts_[udpport];
    pts[tcpport] = pts_[tcpport];
}
State::~State() {}

int State::getIP(){
    sockaddr_in server;
    socklen_t size = sizeof(server);
    getsockname(sds[tcpsd], (sockaddr*)&server, &size);
    return server.sin_addr.s_addr;
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

static char intToHex(char digit){
    if (digit<10)
        return '0'+digit;
    if (digit<16)
        return 'a'+digit-10;
    return -1;
}

const char *State::getUUID(){
    static unsigned char in[20], out[20]; ///2*long+short=18. Poate pe viitor trebuie marit
    long ip=getIP(), timestamp=getTicks();
    unsigned short port=udpport, offset=0;
    static char result[41]={};

    memset(in, 0, sizeof(in));

    memcpy(in+offset, &ip, sizeof(ip));
    offset += sizeof(ip);
    memcpy(in+offset, &port, sizeof(port));
    offset += sizeof(port);
    memcpy(in+offset, &timestamp, sizeof(timestamp));
    offset += sizeof(timestamp);

    sha1::calc(in, offset, out);
    for (unsigned i=0; i<sizeof(out); ++i){
        result[2*i] = intToHex(out[i]&15);
        result[2*i+1] = intToHex((out[i]&(15<<4))>>4);
    }
    //combinatie intre IP, port, ticks (sha1)
    return result;
}

#include "State.h"
#include "sha1.h"

State::State() {}
State::~State() {}

int State::getIP(){
    return 0; ///to be implemented in the future if necesary
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
    unsigned short port=getPort(), offset=0;
    static char result[41]={};

    memset(in, 0, sizeof(in));

    printf("[UUID] %ld %ld %d\n", ip, timestamp, port);
    memcpy(in+offset, &ip, sizeof(ip));
    offset += sizeof(ip);
    memcpy(in+offset, &port, sizeof(port));
    offset += sizeof(port);
    memcpy(in+offset, &timestamp, sizeof(timestamp));
    offset += sizeof(timestamp);

    printf("off %d\n", offset);
    sha1::calc(in, offset, out);
    for (unsigned i=0; i<sizeof(out); ++i){
        result[2*i] = intToHex(out[i]&15);
        result[2*i+1] = intToHex((out[i]&(15<<4))>>4);
    }
    //combinatie intre IP, port, ticks (sha1)
    return result;
}

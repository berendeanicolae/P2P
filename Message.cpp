#include "Message.h"
#include <cstring>

int Message::nextPow2(int x){
    --x;
    for (unsigned i=1; i<sizeof(x); i<<=1){
        x |= x>>i;
    }
    ++x;
    return x;
}

void Message::push_back(MSG msg){
    if (!buffer){
        capacity = sizeof(msg);
        buffer = new unsigned char[capacity];
        memcpy(buffer+index, &msg, sizeof(msg));
        index += sizeof(msg);
    }
    else{
        if (index+sizeof(msg)>capacity){
            unsigned char *tempBuffer;

            capacity = nextPow2(index+sizeof(msg));
            tempBuffer = new unsigned char[capacity];
            memcpy(tempBuffer, buffer, index);
            delete[] buffer;
            buffer = tempBuffer;
        }
        memcpy(buffer+index, &msg, sizeof(msg));
        index += sizeof(msg);
    }
}

void Message::push_back(unsigned short size, const void *src){
    if (size<=0 || !src)
        return;

    if (!buffer){
        capacity = nextPow2(size+2);
        buffer = new unsigned char[capacity];
        memcpy(buffer+index, &size, sizeof(size));
        index += sizeof(size);
        memcpy(buffer+index, src, size);
        index += size;
    }
    else{
        if (index+size+2>capacity){
            unsigned char *tempBuffer;

            capacity = nextPow2(index+size+2);
            tempBuffer = new unsigned char[capacity];
            memcpy(tempBuffer, buffer, index);
            delete[] buffer;
            buffer = tempBuffer;
        }
        memcpy(buffer+index, &size, sizeof(size));
        index += sizeof(size);
        memcpy(buffer+index, src, size);
        index += size;
    }
}

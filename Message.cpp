#include "Message.h"
#include <cstring>

/**
    \return Return the next power of 2 >= x
*/
int Message::nextPow2(int x){
    --x;
    for (unsigned i=1; i<sizeof(x); i<<=1){
        x |= x>>i;
    }
    ++x;
    return x;
}

/**
    @param[in] size The size of the buffer
    @param[in] src Pointer to buffer
*/
Message::Message(unsigned size, char *src): capacity(nextPow2(size)), front(0), end(0){
    buffer = new unsigned char[capacity];
    memcpy(buffer+end, src, size);
    end += size;
}

Message::Message(const Message& other): buffer(0), capacity(other.capacity), front(0), end(other.end){
    buffer = new unsigned char[capacity];
    memcpy(buffer, other.buffer, end);
}

Message::Message(Message& other): buffer(0), capacity(other.capacity), front(0), end(other.end){
    buffer = new unsigned char[capacity];
    memcpy(buffer, other.buffer, end);
}

/**
    @param[in] msg MSG to be added to the message. It should be added at the begining of the message
*/
void Message::push_back(MSG msg){
    if (!buffer){
        capacity = sizeof(msg);
        buffer = new unsigned char[capacity];
        memcpy(buffer+end, &msg, sizeof(msg));
        end += sizeof(msg);
    }
    else{
        if (end+sizeof(msg)>capacity){
            unsigned char *tempBuffer;

            capacity = nextPow2(end+sizeof(msg));
            tempBuffer = new unsigned char[capacity];
            memcpy(tempBuffer, buffer, end);
            delete[] buffer;
            buffer = tempBuffer;
        }
        memcpy(buffer+end, &msg, sizeof(msg));
        end += sizeof(msg);
    }
}

/**
    @param[in] size The size of the buffer
    @param[in] src Pointer to buffer
*/
void Message::push_back(unsigned short size, const void *src){
    if (size<=0 || !src)
        return;

    if (!buffer){
        capacity = nextPow2(size+2);
        buffer = new unsigned char[capacity];
        memcpy(buffer+end, &size, sizeof(size));
        end += sizeof(size);
        memcpy(buffer+end, src, size);
        end += size;
    }
    else{
        if (end+size+2>capacity){
            unsigned char *tempBuffer;

            capacity = nextPow2(end+size+2);
            tempBuffer = new unsigned char[capacity];
            memcpy(tempBuffer, buffer, end);
            delete[] buffer;
            buffer = tempBuffer;
        }
        memcpy(buffer+end, &size, sizeof(size));
        end += sizeof(size);
        memcpy(buffer+end, src, size);
        end += size;
    }
}

void Message::pop_front(MSG& msg){
    if (front+sizeof(msg)>end){
        msg = MSG_invalid;
        return;
    }
    memcpy(&msg, &buffer[front], sizeof(msg));
    front += sizeof(msg);
}

unsigned short Message::pop_front(void* dst){
    unsigned short size = getNextSize();

    if (size){
        front += sizeof(size);
        memcpy(dst, &buffer[front], size);
        front += size;
    }
    return size;
}

unsigned short Message::pop_front(char **dst){
    unsigned short size = getNextSize();

    if (size){
        front += sizeof(size);
        *dst = new char[size+1];
        memcpy(*dst, &buffer[front], size);
        (*dst)[size] = 0;
        front += size;
    }
    return size;
}

unsigned short Message::getNextSize(){
    unsigned short size;

    if (front+sizeof(size)>end){
        return 0;
    }
    memcpy(&size, &buffer[front], sizeof(size));
    if (front+sizeof(size)+size>end){
        return 0;
    }
    return size;
}

void Message::clear(){
    capacity = front = end = 0;
    if (buffer){
        delete[] buffer;
        buffer = 0;
    }
}

#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

enum MSG{
    MSG_connectAsPeer,
    MSG_connectAsServer,
    MSG_connectedOK,
    MSG_quit,
    MSG_ping,
    MSG_pong,
    MSG_searchNoIP,
    MSG_search
};

class Message{
private:
    unsigned char *buffer;
    unsigned int capacity, index;

    int nextPow2(int x);
public:
    Message(): buffer(0), capacity(0), index(0) {}
    ~Message() {if (buffer) delete[] buffer;}

    void push_back(MSG msg);
    void push_back(unsigned short size, const void *src);
    const unsigned char *getMessage() {return buffer;}
    unsigned int getSize() {return index;}
};

#endif

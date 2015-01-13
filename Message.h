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
    MSG_search,
    MSG_invalid
};

class Message{
private:
    unsigned char *buffer;
    unsigned capacity, front, end;

    int nextPow2(int x);
public:
    Message(): buffer(0), capacity(0), front(0), end(0) {}
    Message(unsigned size, char *src);
    ~Message() {if (buffer) delete[] buffer;}

    void push_back(MSG msg);
    void push_back(unsigned short size, const void *src);
    void pop_front(MSG& msg);
    unsigned short pop_front(void *dst);
    unsigned short pop_front(char **dst);
    //unsigned short pop_front(char *&dst);
    const unsigned char *getMessage() {return buffer;}
    unsigned short getNextSize(); //returneaza valoarea corecta doar cand nu urmeaza un MSG
    unsigned getSize() {return end;}
    void clear();
};

#endif
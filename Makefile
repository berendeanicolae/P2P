CC=g++
CFLAGS=-c -Wall -std=c++11 -O2 -pthread
LDFLAGS= -Wall -std=c++11 -O2 -pthread
SOURCES=main.cpp Application.cpp State.cpp Client.cpp Server.cpp Time.cpp sha1.cpp File.cpp Message.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(APP_NAME)

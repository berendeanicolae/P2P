all:
	g++ -o main main.cpp Application.cpp State.cpp Client.cpp Server.cpp Time.cpp sha1.cpp File.cpp -Wall -std=c++11

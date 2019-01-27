override CXXFLAGS := -std=c++14 -pedantic -Wall -Wextra $(CXXFLAGS)

all: client server

server: server.cpp utils.hpp sockets.hpp
	${CXX} ${CXXFLAGS} -pthread -o server server.cpp

client: client.cpp utils.hpp sockets.hpp
	${CXX} ${CXXFLAGS} -o client client.cpp

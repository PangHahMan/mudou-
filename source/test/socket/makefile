.PHONY:all
.PHONY:clean
all:server client

server:server.cpp
	g++ -o $@ $^ -std=c++17

client:client.cpp
	g++ -o $@ $^ -std=c++17

clean:
	rm -f server client
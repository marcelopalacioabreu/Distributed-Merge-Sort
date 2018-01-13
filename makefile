all:
	mkdir -p ./build
	g++ -std=c++11 -pthread server.cpp -o ./build/server
	g++ -std=c++11 -pthread client.cpp -o ./build/client

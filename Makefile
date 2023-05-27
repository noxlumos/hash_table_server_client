compile:
	g++ -std=c++17 server.cpp -pthread -lrt -o server
	g++ -std=c++17 client.cpp -pthread -lrt -o client
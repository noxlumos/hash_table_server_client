compile:
	@echo "Compiling server program."
	g++ -std=c++17 server.cpp -pthread -lrt -o server
	@echo "Compiling client program."
	g++ -std=c++17 client.cpp -pthread -lrt -o client
	@echo "Compilation finished!"
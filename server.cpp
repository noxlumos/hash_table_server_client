#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <shared_mutex>
#include <unordered_map>
#include <thread>
#include <vector>
#include "common.h"
#include "hash_table.hpp"

#define MAX_CLIENTS 10

class Server {
private:
    HashTable *hashtable;

public:
    Server(int tableSize) {
        this->hashtable = new HashTable(tableSize);
    }
    
    void processRequest(const Request& request, Response& response, int clientId) {
        switch (request.operation) {
            case 'I':
                insert(request.key, request.value,response, clientId);
                break;
            case 'G':
                get(request.key, response, clientId);
                break;
            case 'D':
                remove(request.key, response, clientId);
                break;
            default:
                std::cout << "Unknown operation" << std::endl;
        }
    }

    void insert(int key, int value, Response& response, int clientId) {

        hashtable->insert(key, value);
        std::cout << "Client " << clientId << " inserted (" << key << ", " << value << ")" << std::endl;
        response.success = true;
    }

    void get(int key, Response& response, int clientId) {
        std::cout << "Client " << clientId << " get key: " << key << std::endl;
        std::pair<bool, int> result = hashtable->get(key);
        if (result.first) {
            response.key = key;
            response.value = result.second;
            response.success = true;
        } else {
            response.success = false;
        }
    }

    void remove(int key, Response& response, int clientId) {

        bool result = hashtable->remove(key);
        if (result) {
            std::cout << "Client " << clientId << " deleted key: " << key << std::endl;
            response.success = true;
        } else {
            std::cout << "Client " << clientId << " key not found: " << key << std::endl;
            response.success = false;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <table_size>" << std::endl;
        return 1;
    }

    int tableSize = std::atoi(argv[1]);

    std::vector<int> shmids;
    std::vector<char*> shmaddrs;

    Server server = Server(tableSize);
    std::vector<std::thread> clientThreads;

    std::cout << "Server started. Initiating client threads..." << std::endl;
    
    for (int clientId = 1; clientId <= MAX_CLIENTS; clientId++) {
        int shmid = shmget(clientId, SHM_SIZE, IPC_CREAT | 0666);
        char* shmaddr = (char*)shmat(shmid, (void*)0, 0);
        std::memset(shmaddr, 0, SHM_SIZE);
        shmids.push_back(shmid);
        shmaddrs.push_back(shmaddr);

        std::thread clientThread([&server, shmaddr, clientId]() {
            while (true) {
                Request request;
                memcpy(&request, shmaddr, sizeof(Request));
                if(request.updated) {
                    Response response;
                    server.processRequest(request, response, clientId);
                    response.updated = true;
                    std::string output = response.success ? "successfull operation" : "failed operation";
                    std::cout << "sending response: " << output << " to client " << clientId << std::endl;
                    memcpy(shmaddr + sizeof(Request), &response, sizeof(Response));
                    std::memset(shmaddr, 0, sizeof(Request));
                }
                sleep(1);
            }
        });

        clientThreads.push_back(std::move(clientThread));
    }

    for (auto& thread : clientThreads) {
        thread.join();
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        shmdt(shmaddrs[i]);
        shmctl(shmids[i], IPC_RMID, NULL);
    }

    return 0;
}
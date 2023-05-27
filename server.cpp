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

#define MAX_CLIENTS 10

class Server {

public:
    std::unordered_map<int, std::unordered_map<int, int>> hashTables_;
    std::shared_mutex mutex_;
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
        std::unique_lock<std::shared_mutex> lock(mutex_);
        hashTables_[clientId][key] = value;
        std::cout << "Client " << clientId << " inserted (" << key << ", " << value << ")" << std::endl;
        response.success = true;
    }

    void get(int key, Response& response, int clientId) {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto& hashTable = hashTables_[clientId];
        auto it = hashTable.find(key);
        if (it != hashTable.end()) {
            response.key = key;
            response.value = it->second;
            response.success = true;
        } else {
            response.success = false;
        }
    }

    void remove(int key, Response& response, int clientId) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto& hashTable = hashTables_[clientId];
        auto it = hashTable.find(key);
        if (it != hashTable.end()) {
            hashTable.erase(it);
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

    key_t key = ftok("shmfile", 65);

    Server server;
    std::vector<std::thread> clientThreads;
    

    for (int clientId = 0; clientId < MAX_CLIENTS; ++clientId) {
        int shmid = shmget(key + clientId, SHM_SIZE, IPC_CREAT | 0666);
        char* shmaddr = (char*)shmat(shmid, (void*)0, 0);
        //td::memset(shmaddr, 0, sizeof(Request)+sizeof(Response));

        shmids.push_back(shmid);
        shmaddrs.push_back(shmaddr);

        std::thread clientThread([&server, shmaddr, clientId]() {
            while (true) {
                if (shmaddr[0] != '\0') {
                    Request request;
                    memcpy(&request, shmaddr, sizeof(Request));

                    Response response;
                    std::cout << "process request: "<< request.operation << std::endl;
                    server.processRequest(request, response, clientId);
                    std::cout << "sending response: "<< response.success << std::endl;

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
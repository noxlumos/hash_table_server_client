#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "common.h"


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <client_id>" << std::endl;
        return 1;
    }

    int client_id = std::atoi(argv[1]);
    key_t key = ftok("shmfile", 65);
    int shmid = shmget(key + client_id, SHM_SIZE, 0666);
    char* shmaddr = (char*)shmat(shmid, (void*)0, 0);

    while (true) {
        std::cout << "Enter operation (I for insert, G for get, D for delete): " << std::endl;
        char operation;
        std::cin >> operation;

        Request request;
        request.operation = operation;

        if (operation == 'I' || operation == 'G' || operation == 'D') {
            std::cout << "Enter key: "<< std::endl;
            std::cin >> request.key;

            if (operation == 'I') {
                std::cout << "Enter value: "<< std::endl;
                std::cin >> request.value;
            }

            memcpy(shmaddr, &request, sizeof(Request));
            std::cout << "Waiting for response"<< std::endl;
            while (true) {
                if (shmaddr[sizeof(Request)] != '\0') {
                    Response response;
                    memcpy(&response, shmaddr + sizeof(Request), sizeof(Response));
                    std::memset(shmaddr + sizeof(Request), 0, sizeof(Response));
                    std::cout << "response: "<< response.success << std::endl;
                    if (operation == 'G' && response.success) {
                        std::cout << "retrived value: "<< response.value << std::endl;
                    }
                    break;
                }
                sleep(1);
            }
        } else {
            std::cout << "Unknown operation" << std::endl;
        }

        sleep(1);
    }

    shmdt(shmaddr);

    return 0;
}
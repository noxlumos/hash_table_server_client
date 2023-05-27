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
    if (client_id <= 0 || client_id > MAX_CLIENTS) {
        std::cerr << "<client_id> out of range. It should be [1,10]. Exiting." << std::endl;
        exit(1);
    }

    int shmid = shmget(client_id, SHM_SIZE, 0666);
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
            request.updated = true;
            memcpy(shmaddr, &request, sizeof(Request));
            std::cout << "Waiting for response"<< std::endl;
            while (true) {
                Response response;
                memcpy(&response, shmaddr + sizeof(Request), sizeof(Response));
                if(response.updated) {
                    std::string output = response.success ? "successfull operation" : "failed operation";    
                    std::cout << "retrieved message: "<< output<< std::endl;
                    if (operation == 'G' && response.success) {
                        std::cout << "retrived value: "<< response.value << std::endl;
                    }
                    std::memset(shmaddr + sizeof(Request), 0, sizeof(Response));
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
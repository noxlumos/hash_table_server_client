struct Request {
    bool updated;
    char operation;
    int key;
    int value;
};

struct Response {
    bool updated;
    bool success;
    int key;
    int value;
};

int SHM_SIZE = sizeof(Request) + sizeof(Response);

int MAX_CLIENTS = 10;
struct Request {
    char operation;
    int key;
    int value;
};

struct Response {
    int key;
    int value;
    bool success;
};

int SHM_SIZE = sizeof(Request) + sizeof(Response);
#include <iostream>
#include <vector>
#include <shared_mutex>

// Node structure for the linked list
struct Node {
    int key;
    int value;
    Node* next;

    Node(int key, int value) : key(key), value(value), next(nullptr) {}
};

// HashTable class
class HashTable {
private:
    int tableSize;
    std::vector<Node*> table;
    std::vector<std::shared_mutex> locks;  // Readers-writer locks for each bucket

    // Hash function
    int hashFunction(int key) {
        return key % tableSize;
    }

public:

    HashTable() {}
    // Constructor
    HashTable(int size) : tableSize(size), table(size, nullptr), locks(size) {}

    // Destructor
    ~HashTable() {
        for (int i = 0; i < tableSize; ++i) {
            Node* current = table[i];
            while (current != nullptr) {
                Node* temp = current;
                current = current->next;
                delete temp;
            }
        }
    }

    // Insert an item into the hash table
    void insert(int key, int value) {
        int index = hashFunction(key);

        std::unique_lock<std::shared_mutex> lock(locks[index]);

        // Check if the key already exists
        Node* current = table[index];
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;  // Update the value if the key already exists
                return;
            }
            current = current->next;
        }

        // Create a new node
        Node* newNode = new Node(key, value);
        newNode->next = table[index];
        table[index] = newNode;
    }

    // Get the value associated with a key
    std::pair<bool, int> get(int key) {
        int index = hashFunction(key);

        std::shared_lock<std::shared_mutex> lock(locks[index]);

        // Search for the key in the linked list
        Node* current = table[index];
        while (current != nullptr) {
            if (current->key == key) {
                return std::make_pair(true, current->value);
            }
            current = current->next;
        }

        return std::make_pair(false, -1);;  // Return -1 if the key is not found
    }

    // Delete an item from the hash table
    bool remove(int key) {
        int index = hashFunction(key);

        std::unique_lock<std::shared_mutex> lock(locks[index]);

        Node* current = table[index];
        Node* prev = nullptr;

        while (current != nullptr) {
            if (current->key == key) {
                if (prev == nullptr) {
                    // Key is in the first node of the linked list
                    table[index] = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                return true;
            }

            prev = current;
            current = current->next;
        }
        return false;
    }
};

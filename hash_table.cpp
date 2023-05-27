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
    int get(int key) {
        int index = hashFunction(key);

        std::shared_lock<std::shared_mutex> lock(locks[index]);

        // Search for the key in the linked list
        Node* current = table[index];
        while (current != nullptr) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }

        return -1;  // Return -1 if the key is not found
    }

    // Delete an item from the hash table
    void remove(int key) {
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
                return;
            }

            prev = current;
            current = current->next;
        }
    }
};

int main() {
    HashTable hashTable(10);  // Create a hash table of size 10

    // Insert items into the hash table
    hashTable.insert(1, 10);
    hashTable.insert(2, 20);
    hashTable.insert(3, 30);
    hashTable.insert(4, 40);
    hashTable.insert(5, 50);
    hashTable.insert(15, 60);

    // Retrieve values from the hash table
    std::cout << hashTable.get(1) << std::endl;  // Output: 10
    std::cout << hashTable.get(3) << std::endl;  // Output: 30
    std::cout << hashTable.get(5) << std::endl;  // Output: 50

    // Delete an item from the hash table
    hashTable.remove(3);

    std::cout << hashTable.get(3) << std::endl;  // Output: -1 (key not found)
    std::cout << hashTable.get(15) << std::endl;  // Output: -1 (key not found)
    return 0;
}
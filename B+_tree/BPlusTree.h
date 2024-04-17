#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Node {
public:
    vector<int> keys;
    vector<void*> pointers;  // Holds Node* if internal, string* if leaf
    Node* parent;
    Node* next;  // Used for leaves to point to the next leaf
    bool isLeaf;

    Node(bool isLeaf);
    ~Node();
};

class BPlusTree {
private:
    Node* root;
    int maxKeys;

    void insertIntoInterior(Node* n, int key, Node* leftChild, Node* rightChild);
    void insertIntoLeaf(Node* leaf, int key, const string& value);
    void splitLeaf(Node* leaf);
    void splitInterior(Node* node);
    Node* findLeaf(int key);
    void adjustTreeAfterRemoval(Node* node);
    void mergeNodes(Node* left, Node* right);

    void destroyTree(Node* node);
    void copyNodes(Node* toNode, const Node* fromNode);

public:
    BPlusTree(int maxKeys);
    ~BPlusTree();
    bool insert(int key, const string& value);
    bool remove(int key);
    string find(int key);
    void printKeys();
    void printValues();

    // Copy constructor and assignment operator
    BPlusTree(const BPlusTree& other);
    BPlusTree& operator=(const BPlusTree& other);
};
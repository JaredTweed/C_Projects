#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "BPlusTree.h"

using namespace std;

// Node initialization
Node::Node(bool isLeaf) : 
    parent(nullptr), 
    next(nullptr), 
    isLeaf(isLeaf) 
{}

// Node deletion
Node::~Node() {
    for (auto& ptr : pointers) {
        if (ptr) {
            if (isLeaf) {
                delete static_cast<string*>(ptr);
            } else {
                delete static_cast<Node*>(ptr);
            }
        }
    }
}


// B+ tree initialization
BPlusTree::BPlusTree(int maxKeys) : root(nullptr), maxKeys(maxKeys) {}

// B+ tree destructor
BPlusTree::~BPlusTree() {
    destroyTree(root);
}
void BPlusTree::destroyTree(Node* node) {
    if (!node) return;

    // If it's not a leaf, first destroy all children
    if (!node->isLeaf) {
        for (auto& ptr : node->pointers) {
            destroyTree(static_cast<Node*>(ptr));
        }
    }
    // Now it's safe to delete the node itself
    delete node;
}

// B+ tree copy constructor
BPlusTree::BPlusTree(const BPlusTree& other) : root(nullptr), maxKeys(other.maxKeys) {
    if (!other.root) return;
    this->root = new Node(other.root->isLeaf);
    copyNodes(this->root, other.root);
}

// B+ tree assignment operator
BPlusTree& BPlusTree::operator=(const BPlusTree& other) {
    if (this == &other) return *this;  // Self-assignment check

    // Clean up current tree
    destroyTree(this->root);

    // Copy the other tree
    this->maxKeys = other.maxKeys;
    if (other.root) {
        this->root = new Node(other.root->isLeaf);
        copyNodes(this->root, other.root);
    } else {
        this->root = nullptr;
    }

    return *this;
}

// Recursive function to deep-copy nodes
void BPlusTree::copyNodes(Node* toNode, const Node* fromNode) {
    toNode->keys = fromNode->keys;

    if (fromNode->isLeaf) {
        for (const auto& ptr : fromNode->pointers) {
            toNode->pointers.push_back(new string(*static_cast<string*>(ptr)));
        }
    } else {
        for (const auto& ptr : fromNode->pointers) {
            Node* childNode = new Node(static_cast<Node*>(ptr));
            toNode->pointers.push_back(childNode);
            childNode->parent = toNode;
            copyNodes(childNode, static_cast<Node*>(ptr));
        }
    }
}

bool BPlusTree::insert(int key, const string& value) {
    // Create root with value and return true if tree is empty
    if (!root) {
        root = new Node(true);
        root->keys.push_back(key);
        root->pointers.push_back(new string(value));
        return true;
    }

    // Find the leaf with the designated key
    Node* leaf = findLeaf(key);

    // Return false if the key already exists in the leaf
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (key == leaf->keys[i]) {
            // *static_cast<string*>(leaf->pointers[i]) = value; // Reassign value
            return false;
        }
    }

    // Insert into the leaf node
    insertIntoLeaf(leaf, key, value);
        
    // Split the leaf if necessary
    if (leaf->keys.size() > maxKeys) {
        splitLeaf(leaf);
    }
    
    return true;
}

Node* BPlusTree::findLeaf(int key) {
    Node* node = root;

       
    // While not a leaf, set node to the corresponding pointer
    while(!node->isLeaf){
        bool continueWhile = false;
        
        if(key < node->keys[0]){
            node = static_cast<Node*>(node->pointers[0]);
            continue;
        }
        
        for (int i = 1; i < node->keys.size(); i++) {
            if (node->keys[i-1] <= key && key < node->keys[i]) {
                node = static_cast<Node*>(node->pointers[i]);
                continueWhile = true;
                break;
            }            
        }
        if(continueWhile){continue;}

        if(node->keys[node->keys.size()-1] <= key){
            node = static_cast<Node*>(node->pointers[node->keys.size()]);
            continue;
        }
        
    }  

    return node;
}

void BPlusTree::insertIntoLeaf(Node* leaf, int key, const string& value) {
    int i = 0;
    while (i < leaf->keys.size() && key > leaf->keys[i]) {i++;}

    // Place the key after the i-th element and before the (i+1)th element
    leaf->keys.insert(leaf->keys.begin() + i, key);
    leaf->pointers.insert(leaf->pointers.begin() + i, new string(value));
}

int ceilDivide(int a, int b) {
    // Equivalent to ⌈a / b⌉
    return (a + b - 1) / b;
}



void BPlusTree::splitLeaf(Node* leaf) {
    Node* newLeaf = new Node(true);
    int leftLeafSize = ceilDivide(maxKeys + 1, 2);
        
    // Push all keys and records after left key to the new leaf
    for (int i = leftLeafSize; i < leaf->keys.size(); i++) {
        newLeaf->keys.push_back(leaf->keys[i]);
        newLeaf->pointers.push_back(new string(*(static_cast<string*>(leaf->pointers[i]))));
    }

    // Shorten the original node
    while (leaf->keys.size() > leftLeafSize) {
        leaf->keys.pop_back();
        leaf->pointers.pop_back();
    }

    // Redefine pointers
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    newLeaf->parent = leaf->parent;

    // Insert the new key into the parent
    insertIntoInterior(leaf->parent, newLeaf->keys.front(), leaf, newLeaf);
}




void BPlusTree::insertIntoInterior(Node* node, int key, Node* leftChild, Node* rightChild) {
    // Create parent if none exist
    if (!node) {
        root = new Node(false);
        root->keys.push_back(key);
        root->pointers.push_back(leftChild);
        root->pointers.push_back(rightChild);
        leftChild->parent = root;
        rightChild->parent = root;
    } else {
        // Insert the key and 
        int i = 0;
        while (i < node->keys.size() && key > node->keys[i]) {i++;}
        node->keys.insert(node->keys.begin() + i, key);
        node->pointers.insert(node->pointers.begin() + i + 1, rightChild);     

        rightChild->parent = node;

        // Split interior if too large
        if (node->keys.size() > maxKeys) {
            splitInterior(node);
        }
    }
}

void BPlusTree::splitInterior(Node* interior) {
    Node* newInterior = new Node(false);
    int middleIndex = ceilDivide(maxKeys + 1, 2);


    // Push all children after midKey to the new node
    for (int i = middleIndex + 1; i < interior->keys.size(); i++) {
        newInterior->keys.push_back(interior->keys[i]);
        newInterior->pointers.push_back(interior->pointers[i]);
    }
    newInterior->pointers.push_back(interior->pointers.back()); // including the last child


    // Ensure child nodes point back to the correct parent node
    for (auto ptr : newInterior->pointers) {
        ((Node*)ptr)->parent = newInterior;
    }

    // Shorten the original node
    while (interior->keys.size() > middleIndex) {
        interior->keys.pop_back();
        interior->pointers.pop_back();
    }


    // Insert the middle key into the parent, along with the new node pointer
    insertIntoInterior(interior->parent, interior->keys[middleIndex], interior, newInterior);
}


string BPlusTree::find(int key) {
    // Starting from the root, find the leaf node that may contain the key
    Node* leaf = findLeaf(key);

    // Once at the leaf level, check if the key is present
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            return *(static_cast<string*>(leaf->pointers[i])); // Dereference the pointer to return the actual string
        }
    }

    // If the key wasn't found
    return "<empty>";
}

bool BPlusTree::remove(int key) {
    // Start from the root and find the leaf node that may contain the key
    Node* leaf = findLeaf(key);
    int keyIndex = -1;

    // Check if the key is present in the leaf
    for (int i = 0; i < leaf->keys.size(); i++) {
        if (leaf->keys[i] == key) {
            keyIndex = i;
            break;
        }
    }    
    
    // If the key wasn't found, return false
    if (keyIndex == -1) return false;

    // Delete the key and its associated pointer
    delete static_cast<string*>(leaf->pointers[keyIndex]);
    leaf->keys.erase(leaf->keys.begin() + keyIndex);
    leaf->pointers.erase(leaf->pointers.begin() + keyIndex);

    // Adjust the tree if necessary (e.g., merging nodes if underflow occurs)
    adjustTreeAfterRemoval(leaf);

    return true;
}

void BPlusTree::adjustTreeAfterRemoval(Node* node) {
    int minKeys;
    if(node->isLeaf){minKeys = ceilDivide(maxKeys, 2);} // ceiling(maxKeys / 2)
    else {minKeys = maxKeys / 2;} // floor(maxKeys / 2)

    // Base case: if the node is the root or has enough entries, do nothing
    if (node == root || node->keys.size() >= minKeys) return;

    Node* parent = node->parent;
    Node* leftSibling = nullptr;
    Node* rightSibling = nullptr;
    
    // Refers to the key to the right of the pointer to the current node
    int parentKeyIndex = 0;

    // Find the node's index in its parent's pointers
    for (int i = 0; i <= parent->keys.size(); i++) {
        if (parent->pointers[i] == node) {
            parentKeyIndex = i - 1;
            // Assign leftSibling
            if (i > 0) {leftSibling = static_cast<Node*>(parent->pointers[i - 1]);}
            if (i < parent->keys.size()) {rightSibling = static_cast<Node*>(parent->pointers[i + 1]);}
            break;
        }
    }

    // Borrow from sibling if possible
    if(node->isLeaf){
        // Borrow from left sibling if it is large enough
        if (leftSibling && leftSibling->keys.size() > minKeys) {
            // Copy the last left sibling item to the start of the node
            node->keys.insert(node->keys.begin(), leftSibling->keys.back());
            node->pointers.insert(node->pointers.begin(), leftSibling->pointers.back());

            // Update the parent key
            parent->keys[parentKeyIndex] = node->keys.front();

            // Remove the borrowed key and pointer from the left sibling
            leftSibling->keys.pop_back();
            leftSibling->pointers.pop_back();

            return;
        }

        // Borrow from right sibling if it is large enough
        if (rightSibling && rightSibling->keys.size() > minKeys) {
            // Copy the first right sibling item to the end of the node
            node->keys.push_back(rightSibling->keys.front());
            node->pointers.push_back(rightSibling->pointers.front());

            // Update the sibling's parent key
            parent->keys[parentKeyIndex + 1] = node->keys.front();

            // Remove the borrowed key and pointer from the right sibling
            rightSibling->keys.erase(rightSibling->keys.begin());
            rightSibling->pointers.erase(rightSibling->pointers.begin());

            return;
        }

    } else {
        // Borrowing from the left sibling for internal nodes
        if (leftSibling && leftSibling->keys.size() > minKeys) {
            // Prepend the shared parent key and the sibling's child
            node->keys.insert(node->keys.begin(), parent->keys[parentKeyIndex]);
            node->pointers.insert(node->pointers.begin(), leftSibling->pointers.back());

            // Update the parent key
            parent->keys[parentKeyIndex] = leftSibling->keys.back();

            // Remove the borrowed key and pointer from the left sibling
            leftSibling->keys.pop_back();
            leftSibling->pointers.pop_back();

            // Reassign parent of the borrowed pointer
            static_cast<Node*>(node->pointers[0])->parent = node;

            return;
        }
        
        // Borrowing from the right sibling for internal nodes
        if (rightSibling && rightSibling->keys.size() > minKeys) {
            // Append the shared parent key and the sibling's child
            node->keys.push_back(parent->keys[parentKeyIndex + 1]);
            node->pointers.push_back(rightSibling->pointers[0]);

            // Update the sibling's parent key
            parent->keys[parentKeyIndex + 1] = rightSibling->keys.front();

            // Remove the borrowed key and pointer from the right sibling
            rightSibling->keys.erase(rightSibling->keys.begin());
            rightSibling->pointers.erase(rightSibling->pointers.begin());

            // Reassign parent of the borrowed pointer
            static_cast<Node*>(node->pointers.back())->parent = node;

            return;
        }
    }

    // If borrowing is not possible, merge with a sibling
    if (leftSibling) {
        mergeNodes(leftSibling, node);
        adjustTreeAfterRemoval(parent);
    } else if (rightSibling) {
        mergeNodes(node, rightSibling);
        adjustTreeAfterRemoval(parent); 
    }
}

void BPlusTree::mergeNodes(Node* leftNode, Node* rightNode) {
    // Find shared parent key index of both nodes.
    Node* parent = leftNode->parent;
    int parentKeyIndex = 0;
    for (int i = 0; i < parent->keys.size(); i++) {
        if (parent->pointers[i] == leftNode) {
            parentKeyIndex = i;
            break;
        }
    }
    
    // Copy data from the right node to the left node
    if (leftNode->isLeaf) {
        for (int i = 0; i < rightNode->keys.size(); i++) {
            leftNode->keys.push_back(rightNode->keys[i]);
            leftNode->pointers.push_back(rightNode->pointers[i]);
        }

        // Update the next pointer of the left node
        leftNode->next = rightNode->next;

    } else { // If nodes are internal nodes
        // Append the shared parent key to the leftNode 
        leftNode->keys.push_back(parent->keys[parentKeyIndex]);

        // Append all keys and pointers form rightNode to leftNode
        for (int i = 0; i < rightNode->keys.size(); i++) {
            leftNode->keys.push_back(rightNode->keys[i]);
            leftNode->pointers.push_back(rightNode->pointers[i]);
        }
        leftNode->pointers.push_back(rightNode->pointers.back());

        // Update the parent pointers of moved children
        for (auto& ptr : leftNode->pointers) {
            static_cast<Node*>(ptr)->parent = leftNode;
        }
    }

    // Remove the shared parent key
    parent->keys.erase(parent->keys.begin() + parentKeyIndex);
    
    // Delete the right node
    parent->pointers.erase(parent->pointers.begin() + parentKeyIndex + 1);
    delete rightNode;
}

// Uses breadth-first-search.
void BPlusTree::printKeys() {
    // Check for an empty tree first
    if (!root) {
        cout << "The tree is empty." << endl;
        return;
    }

    // Use a queue to keep track of nodes to visit
    queue<Node*> nodesToVisit;
    nodesToVisit.push(root);
    
    while (!nodesToVisit.empty()) {
        int currentLevelSize = nodesToVisit.size(); // Number of nodes at the current level
        for (int i = 0; i < currentLevelSize; i++) {
            Node* currentNode = nodesToVisit.front();
            nodesToVisit.pop();
            
            // Output the keys of the current node
            cout << "[";
            for (int j = 0; j < currentNode->keys.size(); j++) {
                cout << currentNode->keys[j];
                if (j != currentNode->keys.size() - 1) {cout << " ";}
            }
            cout << "]";
            
            // If it's not a leaf node, enqueue its children
            if (!currentNode->isLeaf) {
                for (int j = 0; j <= currentNode->keys.size(); j++) {
                    nodesToVisit.push(static_cast<Node*>(currentNode->pointers[j]));
                }
            }

            // Space between nodes of the same level
            if (i != currentLevelSize - 1) {cout << " ";}
        }
        
        // Move to the next level
        cout << endl;
    }
}


void BPlusTree::printValues() {
    if (!root) {
        cout << "The tree is empty." << endl;
        return;
    }

    // Start from the root and go down to the first leaf node (leftmost)
    Node* currentNode = root;
    while (!currentNode->isLeaf) {
        currentNode = static_cast<Node*>(currentNode->pointers[0]);
    }

    // Traverse the leaf nodes
    while (currentNode) {
        for (int i = 0; i < currentNode->pointers.size(); i++) {
            cout << *(static_cast<string*>(currentNode->pointers[i])) << endl;
        }
        currentNode = currentNode->next;
    }
}

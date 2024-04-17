#include <iostream>
#include <vector>
#include "BPlusTree.h"

using namespace std;

// Function Prototypes
void simpleTest();

int main() {
    simpleTest();
    cout << endl;
}

void simpleTest()
{
    BPlusTree bp1(4);

    // Insert, remove and find
    bp1.insert(7, "seven");
    bp1.insert(1, "one");
    bp1.insert(3, "three");
    bp1.insert(9, "nine");
    bp1.insert(5, "five");
    cout << "find 3: " << bp1.find(3) << " (three)" << endl;
    bp1.remove(7);
    cout << "find 7: " << bp1.find(7) << " (<empty>)" << endl << endl;

    // Printing
    bp1.printKeys();
    cout << endl << "CHECK" << endl;
    cout << "[5]" << endl;
    cout << "[1 3] [5 9]" << endl << endl;
    bp1.printValues();
    cout << endl << "CHECK" << endl;
    cout << "one" << endl << "three" << endl << "five" << endl << "nine" << endl;

    // Copy constructor and op=
    BPlusTree bp2(bp1);
    BPlusTree bp3(7);
    bp3.insert(13, "thirteen");
    bp3 = bp1;

    cout << endl << "simple test complete" << endl;
}
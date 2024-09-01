#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>
#include <utility>
#include <map>
#include <set>
#include <cstdlib>

#define ll long long
using namespace std;

// Structure to represent a node in the simulation
struct node
{
    int backofftime;    // Remaining backoff time before node can transmit
    int duration;       // Remaining duration of current transmission
    int numberofbackoff; // Number of times the node had to back off before successful transmission
};

// Function to calculate factorial
int fac(int n) 
{ 
    return (n==1 || n==0) ? 1: n * fac(n - 1);  
}

// Function to handle collision: when multiple nodes try to transmit simultaneously
void handleCollision(node nodes[], int n, int z, int &collision) {
    int tempcolcount = 0;
    for (int i = 0; i < n; i++) {
        if (nodes[i].backofftime == 0) {
            cout << "\nCollision occurred for node: " << i << endl;
            nodes[i].backofftime = rand() % z;
            nodes[i].numberofbackoff++;
            tempcolcount++;
            cout << "New backoff time for node " << i << " is " << nodes[i].backofftime << endl;
        } else {
            nodes[i].backofftime--;
        }
    }
    // Update collision count
    if (tempcolcount != 0) {
        collision += fac(tempcolcount);
    }
}

// Function to execute transmission for a node
void executeNode(int curr, node nodes[], int n, int z, bool &line, int &success, int &blocked, int &collision) {
    if (nodes[curr].duration == 0) {
        // If transmission is complete
        cout << "\nNode " << curr << " executed" << endl;
        success++;
        cout << "Number of times " << curr << " had to back off: " << nodes[curr].numberofbackoff << endl;
        nodes[curr].numberofbackoff = 0;
        nodes[curr].duration = 5;   // Reset duration for next transmission
        cout << "Duration for node " << curr << " reset to " << nodes[curr].duration << endl;
        nodes[curr].backofftime = rand() % z; // Set new backoff time for next transmission
        cout << "New backoff time for node " << curr << " is " << nodes[curr].backofftime << endl;

        // Handle collision after successful transmission
        handleCollision(nodes, n, z, collision);
    } else if (nodes[curr].duration > 0) {
        // If transmission is in progress
        cout << "Currently executing node " << curr << " with duration " << nodes[curr].duration << endl;
        nodes[curr].duration--; // Decrement duration for ongoing transmission
        // Check for other nodes being blocked
        for (int i = 0; i < n; i++) {
            if (i != curr) {
                if (nodes[i].backofftime == 0) {
                    cout << "\n" << i << " blocked as " << curr << " is executing" << endl;
                    blocked++;
                    nodes[i].numberofbackoff++;
                    nodes[i].backofftime = rand() % z;
                    cout << "New backoff time for node " << i << " is " << nodes[i].backofftime << endl;
                } else {
                    nodes[i].backofftime--;
                }
            }
        }
    }
}

int main() {
    int n;
    cout << "Enter total number of nodes: ";
    cin >> n;
    int t;
    cout << "Enter duration for each node: ";
    cin >> t;
    int z;
    cout << "Enter range of backoff time (upper limit) for node: ";
    cin >> z;
    int totalround;
    cout << "Enter total time for simulation: ";
    cin >> totalround;

    struct node nodes[n];
    bool line = false;
    int success = 0, collision = 0, blocked = 0;
    
    // Initialize nodes
    for (int i = 0; i < n; i++) {
        nodes[i].duration = t;
        nodes[i].backofftime = 0;
        nodes[i].numberofbackoff = 0;
    }
    
    int curr;
    // Main simulation loop
    for (int _ = 0; _ < totalround; _++) {
        cout << "============= TIME t = " << _ << "=============" << endl;
        if (!line) {
            int tempcolcount = 0;
            int count = 0;
            int ithnode;
            // Check for collisions
            for (int i = 0; i < n; i++) {
                if (nodes[i].backofftime == 0) {
                    count++;
                    ithnode = i;
                }
            }
            if (count > 1) {
                handleCollision(nodes, n, z, collision);
            }
            if (count == 1) {
                line = true;
                cout << "Node " << ithnode << " is transmitting with remaining time " << nodes[ithnode].duration << endl;
                nodes[ithnode].duration--;
                curr = ithnode;
            }
            if (count == 0) {
                for (int i = 0; i < n; i++) {
                    nodes[i].backofftime--;
                }
            }
        } else {
            // Execute transmission for the current node
            executeNode(curr, nodes, n, z, line, success, blocked, collision);
        }
    }

    // Print simulation statistics
    cout << "========   STATISTICS   ========" << endl;
    cout << "Number of Successful transmissions: " << success << endl;
    cout << "Number of Collisions: " << collision << endl;
    cout << "Number of times nodes were blocked: " << blocked << endl;

    return 0;
}

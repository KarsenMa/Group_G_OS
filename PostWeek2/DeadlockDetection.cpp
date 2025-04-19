/*
  Group: G
  Author: Clayton Nunle
  Email: clayton.nunley@okstate.edu
  Date: 04-12-2025
 
  Description: Deadlock detection for railway simulation using resource allocation graph.
*/

#include "shared_Mem.h"
#include "Resource_Allocation.h"
#include "DeadlockResolution.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

using namespace std;

// Structure to represent a node in the resource allocation graph
struct Node {
    string id;  // either trainID ("Train0") or intersectionID ("IntersectionA")
    bool isTrain;  // true if node represents a train, false if intersection
    vector<string> edges;  // outgoing edges to other nodes
};

class DeadlockDetector {
private:
    unordered_map<string, Node> graph;
    unordered_set<string> visited;
    unordered_set<string> recStack;

public:
    // Build the resource allocation graph from shared memory
    void buildGraph(shared_mem_t *shm, const vector<Intersection> &intersections) {
        graph.clear();
        
        // Access the held matrix
        int *held = reinterpret_cast<int *>(
            reinterpret_cast<char *>(shm) + sizeof(shared_mem_t) +
            shm->num_sem * sizeof(int) +
            shm->num_mutex * sizeof(pthread_mutex_t) +
            shm->num_sem * sizeof(sem_t));
        
        // Access the waiting matrix
        int *waiting = held + (shm->num_trains * shm->num_intersections);
        
        // Create nodes for all trains and intersections
        for (int t = 0; t < shm->num_trains; t++) {
            string trainID = "Train" + to_string(t);
            Node trainNode;
            trainNode.id = trainID;
            trainNode.isTrain = true;
            graph[trainID] = trainNode;
        }
        
        for (int i = 0; i < shm->num_intersections; i++) {
            string intersectionID = intersections[i].name;
            Node intersectionNode;
            intersectionNode.id = intersectionID;
            intersectionNode.isTrain = false;
            graph[intersectionID] = intersectionNode;
        }
        
        // Add edges for held resources (Train -> Intersection)
        for (int t = 0; t < shm->num_trains; t++) {
            string trainID = "Train" + to_string(t);
            
            for (int i = 0; i < shm->num_intersections; i++) {
                if (held[t * shm->num_intersections + i] == 1) {
                    string intersectionID = intersections[i].name;
                    graph[trainID].edges.push_back(intersectionID);
                }
            }
        }
        
        // Add edges for waiting resources (Intersection -> Train)
        for (int t = 0; t < shm->num_trains; t++) {
            string trainID = "Train" + to_string(t);
            
            for (int i = 0; i < shm->num_intersections; i++) {
                if (waiting[t * shm->num_intersections + i] == 1) {
                    string intersectionID = intersections[i].name;
                    graph[intersectionID].edges.push_back(trainID);
                }
            }
        }
    }
    
    // Detect cycles in the graph using DFS
    bool isCyclic(string nodeID, vector<string> &cycle) {
        if (recStack.find(nodeID) != recStack.end()) {
            // Found a cycle, complete it
            cycle.push_back(nodeID);
            return true;
        }
        
        if (visited.find(nodeID) != visited.end()) {
            return false;
        }
        
        visited.insert(nodeID);
        recStack.insert(nodeID);
        cycle.push_back(nodeID);
        
        for (const string &neighbor : graph[nodeID].edges) {
            if (isCyclic(neighbor, cycle)) {
                return true;
            }
        }
        
        // Remove from recursion stack and cycle
        recStack.erase(nodeID);
        cycle.pop_back();
        return false;
    }
    
    // Find a cycle in the resource allocation graph
    bool detectDeadlock(vector<string> &deadlockCycle) {
        visited.clear();
        recStack.clear();
        
        // Check for cycles starting from each train
        for (const auto &pair : graph) {
            if (pair.second.isTrain && visited.find(pair.first) == visited.end()) {
                vector<string> cycle;
                if (isCyclic(pair.first, cycle)) {
                    // Find the start of the cycle
                    string startNode = cycle.back();
                    
                    // Extract the actual cycle
                    bool foundStart = false;
                    for (const string &node : cycle) {
                        if (node == startNode) {
                            foundStart = true;
                        }
                        
                        if (foundStart) {
                            deadlockCycle.push_back(node);
                        }
                    }
                    
                    return true;
                }
            }
        }
        
        return false;
    }
    
    // Debug function to print the graph if needed
    void printGraph() {
        cout << "Resource Allocation Graph:" << endl;
        for (const auto &pair : graph) {
            const Node &node = pair.second;
            cout << node.id << " (" << (node.isTrain ? "Train" : "Intersection") << ") -> ";
            
            if (node.edges.empty()) {
                cout << "None";
            } else {
                for (size_t i = 0; i < node.edges.size(); i++) {
                    cout << node.edges[i];
                    if (i < node.edges.size() - 1) {
                        cout << ", ";
                    }
                }
            }
            cout << endl;
        }
    }
};

// Format a cycle into a readable string
string formatCycle(const vector<string> &cycle) {
    string result = "";
    for (size_t i = 0; i < cycle.size(); i++) {
        result += cycle[i];
        if (i < cycle.size() - 1) {
            result += " → ";
        }
    }
    return result;
}

// Function to check for deadlocks in the railway system
bool checkForDeadlock(shared_mem_t *shm, const vector<Intersection> &intersections, string &cycleDesc) {
    DeadlockDetector detector;
    
    // Build the resource allocation graph
    detector.buildGraph(shm, intersections);
    
    // For debugging
    // detector.printGraph();
    
    // Detect deadlock
    vector<string> deadlockCycle;
    bool hasDeadlock = detector.detectDeadlock(deadlockCycle);
    
    if (hasDeadlock) {
        cycleDesc = formatCycle(deadlockCycle);
        return true;
    }
    
    return false;
}

// Helper function to resolve deadlocks by selecting a train to preempt
string selectTrainToPreempt(const vector<string> &deadlockCycle) {
    // Simple strategy: select the first train in the cycle
    for (const string &node : deadlockCycle) {
        if (node.substr(0, 5) == "Train") {
            return node;
        }
    }
    return "";
}

// Function to find an intersection that a train holds
string getIntersectionHeldByTrain(shared_mem_t *shm, const vector<Intersection> &intersections, 
                                 const string &trainID) {
    // Extract train number
    int trainNum = stoi(trainID.substr(5));
    
    // Access the held matrix
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(shm) + sizeof(shared_mem_t) +
        shm->num_sem * sizeof(int) +
        shm->num_mutex * sizeof(pthread_mutex_t) +
        shm->num_sem * sizeof(sem_t));
    
    // Find an intersection held by this train
    for (int i = 0; i < shm->num_intersections; i++) {
        if (held[trainNum * shm->num_intersections + i] == 1) {
            return intersections[i].name;
        }
    }
    
    return "";
}

// Function to detect and handle deadlocks

/* 
************
// call this function from main with the shared mem pointer and vector<Intersection> to create graph and run deadlock detection
************
*/
void detectAndResolveDeadlock(shared_mem_t *shm, const vector<Intersection> &intersections) {
    string cycleDescription;
    
    if (checkForDeadlock(shm, intersections, cycleDescription)) {
        cout << "Deadlock detected! Cycle: " << cycleDescription << endl;

        /*
        ******************
        Future Use: Week 4 Task
        ******************
        */
        // Parse the deadlock cycle into a vector
        vector<string> cycle;
        size_t pos = 0;
        string token;
        string delimiter = " → ";
        string cycleString = cycleDescription;
        
        while ((pos = cycleString.find(delimiter)) != string::npos) {
            token = cycleString.substr(0, pos);
            cycle.push_back(token);
            cycleString.erase(0, pos + delimiter.length());
        }
        cycle.push_back(cycleString); // Add the last part
        
        // Select a train to preempt
        string trainToPreempt = selectTrainToPreempt(cycle);
        
        if (!trainToPreempt.empty()) {
            // Find an intersection held by this train
            string intersectionToRelease = getIntersectionHeldByTrain(shm, intersections, trainToPreempt);
            
            if (!intersectionToRelease.empty()) {
                cout << "Preempting " << intersectionToRelease << " from " << trainToPreempt << "." << endl;
                
                // calls to resolveDeadlock in DeadlockResolution.cpp to forcibly release a held intersection
                resolveDeadlock(shm, intersections, trainToPreempt.c_str(), intersectionToRelease.c_str());
            }
        }

    } else {
        cout << "No deadlock detected." << endl;
    }
}

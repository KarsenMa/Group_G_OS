/*
  Group: G
  Author: Clayton Nunley
  Email: clayton.nunley@okstate.edu
  Date: April 13, 2025
 
  Description: Header file for deadlock detection using resource allocation graph.
*/

#ifndef DEADLOCK_DETECTION_H
#define DEADLOCK_DETECTION_H

#include "shared_Mem.h"
#include "Resource_Allocation.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

// Structure to represent a node in the resource allocation graph
struct Node {
    std::string id;  // either trainID ("Train0") or intersectionID ("IntersectionA")
    bool isTrain;    // true if node represents a train, false if intersection
    std::vector<std::string> edges;  // outgoing edges to other nodes
};

// Class to handle deadlock detection using a resource allocation graph
class DeadlockDetector {
private:
    std::unordered_map<std::string, Node> graph;
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recStack;

public:
    // Build the resource allocation graph from shared memory
    void buildGraph(shared_mem_t *shm, const std::vector<Intersection> &intersections);
    
    // Detect cycles in the graph using DFS (returns true if cycle found)
    bool isCyclic(std::string nodeID, std::vector<std::string> &cycle);
    
    // Find a cycle in the resource allocation graph
    bool detectDeadlock(std::vector<std::string> &deadlockCycle);
    
    // Debug function to print the graph
    void printGraph();
};

// Format a cycle into a readable string
std::string formatCycle(const std::vector<std::string> &cycle);

// Function to check for deadlocks in the railway system
bool checkForDeadlock(shared_mem_t *shm, const std::vector<Intersection> &intersections, 
                     std::string &cycleDesc);

// Main function to detect deadlocks
// call this function in main
void detectAndResolveDeadlock(shared_mem_t *shm, const std::vector<Intersection> &intersections);


#endif // DEADLOCK_DETECTION_H

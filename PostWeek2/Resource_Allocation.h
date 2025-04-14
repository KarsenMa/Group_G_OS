/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

#ifndef RESOURCE_ALLOC_H
#define RESOURCE_ALLOC_H

#include "shared_Mem.h"
#include <string>
#include <vector>
#include <unordered_map>

// Struct to represent an intersection (name, type, capacity)
struct Intersection
{
    std::string name;
    std::string type; // "Mutex" or "Semaphore"
    int index; // intersection number in held matrix
    union{
        int sem_index;
        int mutex_index;
    };
    int capacity;
};

// Parses intersections.txt and fills the vector of Intersection structs
void parseIntersections(const std::string &filename, std::vector<Intersection> &intersections);

// Parses trains.txt and maps train IDs to their ordered list of intersections
void parseTrains(const std::string &filename, std::unordered_map<int, std::vector<std::string>> &trainRoutes);

// Displays the current Resource Allocation Table using shared memory
void printIntersectionStatus(shared_mem_t *shm, const std::vector<Intersection> &intersections);

#endif

/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

#include "shared_Mem.h"
#include "ResourceAlloc.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <string>
#include <cstring>

using namespace std;

// Struct to represent each intersection
// struct Intersection
// {
//     string name;
//     string type;  // "Mutex" or "Semaphore"
//     int capacity; // 1 for Mutex, >1 for Semaphore
// };

// Parse intersections.txt to fill a vector of Intersection structs
void parseIntersections(const string &filename, vector<Intersection> &intersections)
{
    ifstream file(filename);
    string line;
    while (getline(file, line))
    {
        size_t colon = line.find(':');
        if (colon == string::npos)
            continue;

        string name = line.substr(0, colon);
        int cap = stoi(line.substr(colon + 1));
        string type = (cap == 1) ? "Mutex" : "Semaphore";
        intersections.push_back({name, type, cap});
    }
}

// Parse trains.txt to map train IDs to their route of intersections
void parseTrains(const string &filename, unordered_map<int, vector<string>> &trainRoutes)
{
    ifstream file(filename);
    string line;
    while (getline(file, line))
    {
        size_t colon = line.find(':');
        if (colon == string::npos)
            continue;

        string trainLabel = line.substr(0, colon); // e.g., Train0
        int trainID = stoi(trainLabel.substr(5));  // extract '0' from "Train0"
        string routeData = line.substr(colon + 1);

        stringstream ss(routeData);
        string intersection;
        while (getline(ss, intersection, ','))
        {
            trainRoutes[trainID].push_back(intersection);
        }
    }
}

// Display the resource allocation table from shared memory
void printIntersectionStatus(shared_mem_t *shm, const vector<Intersection> &intersections)
{
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(shm) + sizeof(shared_mem_t) +
        shm->num_sem * sizeof(int) +
        shm->num_mutex * sizeof(pthread_mutex_t) +
        shm->num_sem * sizeof(sem_t) + 
        (shm->num_sem + shm->num_mutex) * sizeof(Intersection));

    cout << left << setw(15) << "IntersectionID"
         << setw(10) << "Type"
         << setw(10) << "Capacity"
         << setw(12) << "Lock State"
         << "Holding Trains" << endl;

    cout << string(60, '-') << endl;

    for (int i = 0; i < shm->num_intersections; ++i)
    {
        string lockState = "Unlocked";

        for (int t = 0; t < shm->num_trains; ++t)
        {
            if (held[t * shm->num_intersections + i] == 1)
            {
                lockState = "Locked";
                break;
            }
        }

        cout << left << setw(15) << intersections[i].name
             << setw(10) << intersections[i].type
             << setw(10) << intersections[i].capacity
             << setw(12) << lockState
             << "[";

        bool first = true;
        for (int t = 0; t < shm->num_trains; ++t)
        {
            if (held[t * shm->num_intersections + i] == 1)
            {
                if (!first)
                    cout << ", ";
                cout << "Train" << t;
                first = false;
            }
        }

        cout << "]" << endl;
    }
}

// future use function to create resource allocation table within main function
/*
void createResourceAllocationTable(shared_mem_t *shm, int *held, int* inter_ptr, const vector<Intersection> &intersections, unordered_map<int, vector<string>> &trainRoutes){
    // --- Simulated usage (remove for actual use) ---
    held[0 * num_intersections + 0] = 1; // Train0 holds IntersectionA
    held[1 * num_intersections + 1] = 1; // Train1 holds IntersectionB
    held[2 * num_intersections + 1] = 1; // Train2 also holds IntersectionB
    held[3 * num_intersections + 2] = 1; // Train3 holds IntersectionC

    // Display the current allocation table
    printIntersectionStatus(shm, intersections);



}
*/

int main()
{
    // Parse config files
    vector<Intersection> intersections;
    unordered_map<int, vector<string>> trainRoutes;

    parseIntersections("intersections.txt", intersections);
    parseTrains("trains.txt", trainRoutes);

    int num_trains = trainRoutes.size();
    int num_intersections = intersections.size();

    // Count how many mutexes and semaphores, and prepare sem_values
    int num_mutex = 0;
    vector<int> sem_values_vec;

    for (const auto &inter : intersections)
    {
        if (inter.type == "Mutex")
        {
            num_mutex++;
        }
        else
        {
            sem_values_vec.push_back(inter.capacity);
        }
    }

    int num_sem = sem_values_vec.size();
    int sem_values[num_sem];
    for (int i = 0; i < num_sem; ++i)
    {
        sem_values[i] = sem_values_vec[i];
    }

    // Set up shared memory
    shared_Mem mem;
    void *ptr = mem.mem_setup(num_mutex, num_sem, sem_values, num_trains, num_intersections);
    shared_mem_t *shm = reinterpret_cast<shared_mem_t *>(ptr);

    // Access the held matrix
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(ptr) + sizeof(shared_mem_t) +
        num_sem * sizeof(int) +
        num_mutex * sizeof(pthread_mutex_t) +
        num_sem * sizeof(sem_t));

    // --- Simulated usage (remove for actual use) ---
    held[0 * num_intersections + 0] = 1; // Train0 holds IntersectionA
    held[1 * num_intersections + 1] = 1; // Train1 holds IntersectionB
    held[2 * num_intersections + 1] = 1; // Train2 also holds IntersectionB
    held[3 * num_intersections + 2] = 1; // Train3 holds IntersectionC

    // Display the current allocation table
    printIntersectionStatus(shm, intersections);

    // Clean up
    mem.mem_close(ptr);
    return 0;
}

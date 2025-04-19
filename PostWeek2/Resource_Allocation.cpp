/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

#include "shared_Mem.h"
#include "Resource_Allocation.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <string>
#include <cstring>

using namespace std;

/* No Longer Used */
/*
Struct to represent each intersection
 struct Intersection
 {
     string name;
     string type;
     int capacity;
 };
*/

/* Parse intersections.txt to fill a vector of Intersection structs */
void parseIntersections(const string &filename, vector<Intersection> &intersections)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        cerr << "Failed to open intersections: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {
        size_t colon = line.find(':');
        if (colon == string::npos)
            /* cerr << "Invalid line format: " << line << endl; */
            continue;
        /* cast data to char arrays */
        char name[32];                                              /* create char array for name */
        strncpy(name, line.substr(0, colon).c_str(), sizeof(name)); /* copy name string to char array */
        name[sizeof(name) - 1] = '\0';
        int cap = -1;
        try
        {
            cap = stoi(line.substr(colon + 1)); /* convert string to int */
        }
        catch (const std::invalid_argument &e)
        {
            cerr << "Invalid capacity value in line: " << line << endl;
            continue;
        }

        char type[10]; /* create char array for type */

        if (cap == 1)
        {
            strncpy(type, "Mutex", sizeof(type)); /* if mutex copy "mutex" to type */
        }
        else
        {
            strncpy(type, "Semaphore", sizeof(type)); /* else copy "semaphore" to type */
        }
        type[sizeof(type) - 1] = '\0'; /* null termination */

        Intersection inter;
        strncpy(inter.name, name, sizeof(inter.name)); /* copy name to intersection struct */
        inter.name[sizeof(inter.name) - 1] = '\0';     /* null termination */

        strncpy(inter.type, type, sizeof(inter.type)); /* copy type to intersection struct */
        inter.type[sizeof(inter.type) - 1] = '\0';     /* null termination*/

        inter.index = -1;
        inter.capacity = cap; /* set capacity to intersection struct */
        intersections.push_back(inter);
    }
}

/*Train Parsing no longer used in this file*/
/*void parseTrains()

*/

/* Print Resouce ALlocation Table */
void printIntersectionStatus(shared_mem_t *shm, const vector<Intersection> &intersections)
{
    /* Mem Address of held matrix*/
    char *basePtr = reinterpret_cast<char *>(shm);
    size_t offset = sizeof(shared_mem_t)
        + shm->num_sem * sizeof(int)                             /*Mem for semaphore counter*/
        + shm->num_mutex * sizeof(pthread_mutex_t)               /*Mem for mutexs*/
        + shm->num_sem * sizeof(sem_t)                 /*Mem for semaphores*/
        + (shm->num_sem + shm->num_mutex) * sizeof(Intersection); /*Interseciton data*/
    int *held = reinterpret_cast<int *>(basePtr + offset);

    /*Columns for Resource Allocation Table*/
    cout << left << setw(15) << "IntersectionID"
         << setw(10) << "Type"
         << setw(10) << "Capacity"
         << setw(12) << "Lock State"
         << "Holding Trains" << endl;

    /*Seperate columns from data*/
    cout << string(60, '_') << endl;

    /*Loop through intersections*/
    for (int i = 0; i < shm->num_intersections; ++i)
    {
        /*Lock is not held*/
        string lockState = "Unlocked";

        /*Chech Lock State*/
        for (int t = 0; t < shm->num_trains; ++t)
        {
            if (held[t * shm->num_intersections + i] == 1) /*find intersections held by train*/
            {
                /*If train holds intersection set locked*/
                lockState = "Locked";
                break;
            }
        }

        /*Intersection data*/
        cout << left << setw(15) << intersections[i].name /*name*/
             << setw(10) << intersections[i].type         /*lock type*/
             << setw(10) << intersections[i].capacity     /*inersection capacity*/
             << setw(12) << lockState                     /*Locked/Unlocked*/
             << "[";

        bool one = true;

        /*Loop for printing trains that hold lock on specific intersection*/
        for (int t = 0; t < shm->num_trains; ++t)
        {
            /*True value in held matrix*/
            if (held[t * shm->num_intersections + i] == 1)
            {
                if (!one) /*Multiple elements separate with comma*/
                    cout << ", ";
                cout << "Train" << t; /*Trains Id*/
                one = false;
            }
        }

        cout << "]" << endl;
    }
}

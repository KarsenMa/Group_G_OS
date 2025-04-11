/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

#include "Resource_Allocation.h" //Header for ResourceAllocation
#include <iostream>
#include <iomanip>

// Initialize ResourceAllocation with pointer to shared memory
ResourceAllocation::ResourceAllocation(shared_mem_t *mem)
{
    this->mem = mem;
}

// Set intersection as held
void ResourceAllocation::setHeld(int train_id, int intersection_id)
{
    pthread_mutex_lock(&mem->mutex);                                                  // lock mutex
    mem->held[train_id][intersection_id] = 1;                                         // set held
    std::cout << "Setting held[" << train_id << "][" << intersection_id << "] = 1\n"; // debug

    pthread_mutex_unlock(&mem->mutex); // unlock mutex
}

// Release intersection
void ResourceAllocation::releaseHeld(int train_id, int intersection_id)
{
    pthread_mutex_lock(&mem->mutex);          // lock mutex
    mem->held[train_id][intersection_id] = 0; // release intersection
    pthread_mutex_unlock(&mem->mutex);        // unlock mutex
}

// Set train as waiting
void ResourceAllocation::setWaiting(int train_id, int intersection_id)
{
    pthread_mutex_lock(&mem->mutex);             // lock mutex
    mem->waiting[intersection_id][train_id] = 1; // set waiting
    pthread_mutex_unlock(&mem->mutex);           // unlock mutex
}

// Release waiting
void ResourceAllocation::clearWaiting(int train_id, int intersection_id)
{
    pthread_mutex_lock(&mem->mutex);             // lock mutex
    mem->waiting[intersection_id][train_id] = 0; // release waiting
    pthread_mutex_unlock(&mem->mutex);           // unlock mutex
}

// Print Resource Allocation Table
void ResourceAllocation::printStatus()
{
    pthread_mutex_lock(&mem->mutex); // lock mutex

    std::cout << "\n           RESOURCE ALLOCATION TABLE           \n";

    // Print Held Labels
    std::cout << "\nHeld:\n";
    std::cout << "     ";
    for (int i = 0; i < MAX_INTERSECTIONS; ++i)
    {
        char label = 'A' + i; // Convert index to label
        std::cout << " " << label << "  ";
    }
    std::cout << "\n";
    // Print held table
    for (int t = 1; t < MAX_TRAINS; ++t)
    {
        std::cout << "T" << std::setw(2) << t << "  ";
        for (int i = 0; i < MAX_INTERSECTIONS; ++i)
        {
            std::cout << " " << std::setw(2) << mem->held[t][i] << " "; // Display status of each intersection
        }
        std::cout << "\n";
    }

    // Print Waiting Labels
    std::cout << "\nWaiting:\n";
    std::cout << "     ";
    for (int t = 1; t < MAX_TRAINS; ++t)
    {
        std::cout << "T" << std::setw(2) << t << " ";
    }
    std::cout << "\n";
    // Print waiting table
    for (int i = 0; i < MAX_INTERSECTIONS; ++i)
    {
        char label = 'A' + i;
        std::cout << " " << label << "   ";
        for (int t = 1; t < MAX_TRAINS; ++t)
        {
            std::cout << " " << std::setw(2) << mem->waiting[i][t] << " "; // Display status of each train
        }
        std::cout << "\n";
    }

    std::cout << "\n";
    pthread_mutex_unlock(&mem->mutex); // unlock mutex
}

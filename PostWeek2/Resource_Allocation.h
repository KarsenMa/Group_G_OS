/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

// Ensure this file is only included once
#ifndef Resource_Allocation_h
#define Resource_Allocation_h

// include shared memory from Cosette
#include "shared_Mem.h"

class ResourceAllocation
{
public:
    // Constructor for pointer to shared memory
    ResourceAllocation(shared_mem_t *mem);

    void setHeld(int train_id, int intersection_id);      // Held
    void releaseHeld(int train_id, int intersection_id);  // Released
    void setWaiting(int train_id, int intersection_id);   // Waiting
    void clearWaiting(int train_id, int intersection_id); // Cleared
    void printStatus();                                   // Print Resource Allocation Table

private:
    // Pointer to shared memory
    shared_mem_t *mem;
};

#endif
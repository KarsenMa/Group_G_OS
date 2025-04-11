/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   04-09-2025
*/

#include <iostream>
#include <cstring>
#include "shared_Mem.h"          //Include shared memory header
#include "Resource_Allocation.h" //Include resource allocation header

int main()
{
    // Set up shared memory
    shared_Mem shm;
    shared_mem_t *mem = (shared_mem_t *)shm.mem_setup();

    // Initialize mutex and semaphore
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mem->mutex, &mattr);
    sem_init(&mem->semaphore, 1, 1);

    // Ensure Resource Allocation Table is empty
    memset(mem->held, 0, sizeof(mem->held));
    memset(mem->waiting, 0, sizeof(mem->waiting));

    ResourceAllocation ResourceAlloc(mem);

    // Test Cases
    ResourceAlloc.setHeld(1, 2);    // Train 1 holds intersection C
    ResourceAlloc.setHeld(3, 0);    // Train 3 holds intersection A
    ResourceAlloc.setHeld(4, 1);    // Train 4 holds intersection B
    ResourceAlloc.setHeld(2, 3);    // Train 2 holds intersection D
    ResourceAlloc.setWaiting(2, 3); // Train 2 is waiting on intersection D
    ResourceAlloc.setWaiting(4, 2); // Train 4 is waiting on intersection C
    ResourceAlloc.setWaiting(1, 1); // Train 1 is waiting on intersection B

    // Display Resource Allocation Table
    ResourceAlloc.printStatus();

    // Clean up shared memory
    shm.mem_close(mem);
    return 0;
}

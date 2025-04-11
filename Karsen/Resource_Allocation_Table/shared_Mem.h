/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/3/2025
    Program Description: This file contains methods to create a shared memory to be used to hold
    mutex and semaphore states.
*/

// Added by Karsen for Resource Allocation Table
#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>

// Added by Karsen for Resource Allocation Table
#define MAX_TRAINS 5
#define MAX_INTERSECTIONS 4

// create structure to hold mutex and semaphore
typedef struct
{
    pthread_mutex_t mutex;
    sem_t semaphore;

    // Added by Karsen for Resource Allocation Table
    int held[MAX_TRAINS][MAX_INTERSECTIONS];
    int waiting[MAX_INTERSECTIONS][MAX_TRAINS];

} shared_mem_t;

class shared_Mem
{
public:
    const char *name = "sharedMemory"; // create share memory name for future use and easy access
    void *mem_setup();
    void mem_close(void *ptr);
};

#endif // SHARED_MEM_H
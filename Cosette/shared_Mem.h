/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/3/2025
    Program Description: This file contains methods to create a shared memory to be used to hold
    mutex and semaphore states.
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>

// create structure to hold mutex and semaphore
typedef struct {
    pthread_mutex_t mutex; 
    sem_t semaphore;
} shared_mem_t;

class shared_Mem { 
    public: 
    

        const char *name = "sharedMemory"; // create share memory name for future use and easy access
        void* mem_setup();
        void mem_close(void* ptr);
};



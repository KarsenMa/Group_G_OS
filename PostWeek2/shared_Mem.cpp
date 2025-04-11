/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 3/31/2025
    Program Description: This file contains methods to create a shared memory to be used to hold
    mutex and semaphore states.
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdio>

#include "shared_Mem.h"


void* shared_Mem::mem_setup(int num_mutex, int num_sem){
    
    size_t length = sizeof(shared_mem_t) + num_mutex*sizeof(pthread_mutex_t) + num_sem*sizeof(sem_t); // size of memory object in bytes

    void* mem_ptr; // pointer to memory object


    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);    // creates memory object, set to read and write
    if(shm_fd == -1){
        perror("shm_open error"); // print out error if shared memory is not created
        return nullptr;
    }

    ftruncate(shm_fd, length); // resize the memory to the size of the shared memory structure

    mem_ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // create a pointer to the memory map of the shared memory

    // set shared memory variables to the sizes provided in the mem setup
    mem_ptr->num_mutex = num_mutex;
    mem_ptr->num_sem = num_sem;

    // Create pointers to the mutexes and the semaphores in shared memory
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(mem_ptr + 1); 
    sem_t* semaphore = reinterpret_cast<sem_t*>(mutex + num_mutex);

    pthread_mutexattr_t attribute;
    pthread_mutexattr_init(&attribute);
    pthread_mutexattr_setpshared(&attribute, PTHREAD_PROCESS_SHARED);
    
    // initialize mutexes
    for(int i = 0; i <= num_mutex; i++){ 
        pthread_mutex_init(&mutex[i], &attribute);
    }

    // cleanup setup attribute
    pthread_mutexattr_destroy(&attribute);

    // initialize semaphores
    for(i = 0; i <= num_sem; i++){ 
        sem_init(&semaphore[i], 1, 1);
    }

    return mem_ptr;

}

void shared_Mem::mem_close(void* ptr){
    // unallocated the memory 
    
    // to do: destroy mutex and semaphore objects
    

    munmap(ptr, sizeof(shared_mem_t)); //unmap the memory

    shm_unlink(name); //unlink the shared memory

}

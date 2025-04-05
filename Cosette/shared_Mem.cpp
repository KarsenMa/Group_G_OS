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


void* shared_Mem::mem_setup(){
  

    unsigned int length = sizeof(shared_mem_t); // size of memory object in bytes

    void* mem_ptr; // pointer to memory object


    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);    // creates memory object, set to read and write
    if(shm_fd == -1){
        perror("shm_open error"); // print out error if shared memory is not created
        return nullptr;
    }

    ftruncate(shm_fd, length); // resize the memory to the size of the shared memory structure

    mem_ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // create a pointer to the memory map of the shared memory

    return mem_ptr;

}

void shared_Mem::mem_close(void* ptr){
    // unallocated the memory 

    munmap(ptr, sizeof(shared_mem_t)); //unmap the memory

    shm_unlink(name); //unlink the shared memory

}

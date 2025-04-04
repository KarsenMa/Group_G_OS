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

#include "shared_Mem.h"


void shared_Mem::mem_setup(){
    const char *name = "sharedMemory";
    pthread_mutex_t mutex;
    sem_t semaphore;

    int oflag = O_CREAT | ORDWR; // creates memory object, set to read and write
    mode_t mode  = ; // sets directory permissions of object.

    unsigned int length = 4096; // size of memory object in bytes

    void* mem_ptr; // pointer to memory object


    int shm_fd = shm_open(name, oflag, mode);
    
    ftruncate(shm_fd, length);

    mem_ptr = *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);



}

void shared_Mem::mem_close(int fd, unsigned int length, const char* name){
    // unallocated the memory 
    munmap(fd, length); //unmap the memory

    shm_unlink(name); //unlink the shared memory

}


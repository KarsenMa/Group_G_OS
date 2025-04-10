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
#include <cstdio>
#include <semaphore.h>
#include <unistd.h>
#include "shared_Mem.h"



int main() {

void *ptr;
shared_Mem mem; //create shared memory object
ptr = mem.mem_setup();

shared_mem_t* m = (shared_mem_t*)ptr; // cast ptr to shared_mem struct

// create attribute for mutex to allow for process to share mutex.
pthread_mutexattr_t attribute; 
pthread_mutexattr_init(&attribute);
pthread_mutexattr_setpshared(&attribute, PTHREAD_PROCESS_SHARED);

// initialize mutex with attribute
pthread_mutex_init(&m->mutex, &attribute);


printf("%d\n", pthread_mutex_trylock(&m->mutex)); // will print a 0 if successful
sleep(5);
pthread_mutex_unlock(&m->mutex); // unlock mutex


mem.mem_close(ptr); // close shared memory



}


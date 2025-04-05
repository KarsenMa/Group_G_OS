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
shared_Mem mem;
ptr = mem.mem_setup();

shared_mem_t* m = (shared_mem_t*)ptr; // cast ptr to shared_mem struct

pthread_mutexattr_t attribute;
pthread_mutexattr_init(&attribute);
pthread_mutexattr_setpshared(&attribute, PTHREAD_PROCESS_SHARED);


pthread_mutex_init(&m->mutex, &attribute);

printf("%d", pthread_mutex_trylock(&m->mutex));
sleep(5);
pthread_mutex_unlock(&m->mutex);

mem.mem_close(ptr);



}


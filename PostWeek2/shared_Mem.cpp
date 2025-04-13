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
#include <cstring>

#include "shared_Mem.h"

// mem_setup sets up shared memory object. Num_mutex is the number of mutex objects needed
// sem_values is the vector containing the values that each semaphore needs to be initialized at
// size of sem_values is the number of semaphore objects needed.
void *shared_Mem::mem_setup(int num_mutex, int num_sem, const int sem_values[], int num_trains)
{   
    int num_intersections = num_sem + num_mutex;

    size_t resourceAllocationIntersections = (num_mutex + num_sem) * sizeof(int); // get number of intersections needed

    // size of memory object in bytes
    size_t length = sizeof(shared_mem_t) + num_mutex * sizeof(pthread_mutex_t) + num_sem * sizeof(sem_t) + num_sem * sizeof(int);

    void *mem_ptr; // pointer to memory object

    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666); // creates memory object, set to read and write
    if (shm_fd == -1)
    {
        perror("shm_open error"); // print out error if shared memory is not created
        return nullptr;
    }

    ftruncate(shm_fd, length); // resize the memory to the size of the shared memory structure

    mem_ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // create a pointer to the memory map of the shared memory

    shared_mem_t *mem = static_cast<shared_mem_t *>(mem_ptr); // cast the pointer to the shared memory structure
    // set shared memory variables to the sizes provided in the mem setup
    mem->num_mutex = num_mutex;
    mem->num_sem = num_sem;
    mem->num_trains = num_trains;
    mem->num_intersections = num_intersections;

    // Memory layout: [shared_mem_t][sem_values][mutexes][semaphores][held]
    char *base = reinterpret_cast<char *>(mem_ptr) + sizeof(shared_mem_t);
    int *sem_val_block = reinterpret_cast<int *>(base);
    memcpy(sem_val_block, sem_values, num_sem * sizeof(int));

    // Create pointers to the mutexes and the semaphores in shared memory
    pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(sem_val_block + num_sem);
    sem_t *semaphore = reinterpret_cast<sem_t *>(mutex + num_mutex);

    // create mutex attribute to allow mutex to be accessed by multiple threads/processes
    pthread_mutexattr_t attribute;
    pthread_mutexattr_init(&attribute);
    pthread_mutexattr_setpshared(&attribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mem->rat_mutex, &attribute);

    // initialize mutexes
    for (int i = 0; i < num_mutex; i++)
    {
        pthread_mutex_init(&mutex[i], &attribute);
    }

    // cleanup setup attribute
    pthread_mutexattr_destroy(&attribute);

    // initialize semaphores
    for (int i = 0; i < num_sem; i++)
    {
        sem_init(&semaphore[i], 1, sem_values[i]);
    }

    // Initialize held matrix to 0
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(semaphore) + num_sem * sizeof(sem_t));
    memset(held, 0, num_trains * num_intersections * sizeof(int));

    return mem_ptr;
}

// mem_close closes and unmaps the shared memory, destroys the mutex and semaphore objects
// input is a pointer to shared memory object that needs to be cleared.
void shared_Mem::mem_close(void *ptr)
{

    shared_mem_t *shm = reinterpret_cast<shared_mem_t *>(ptr);

    int num_mutex = shm->num_mutex;
    int num_sem = shm->num_sem;
    int num_trains = shm->num_trains;
    int num_intersections = shm->num_intersections;

    char *base = reinterpret_cast<char *>(ptr) + sizeof(shared_mem_t);
    int *sem_val_block = reinterpret_cast<int *>(base);
    pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(sem_val_block + num_sem);
    sem_t *semaphore = reinterpret_cast<sem_t *>(mutex + num_mutex);
    size_t length = sizeof(shared_mem_t) + num_mutex * sizeof(pthread_mutex_t) + num_sem * sizeof(sem_t) + num_sem * sizeof(int);
    // destroy the mutex objects
    for (int i = 0; i < num_mutex; i++)
    {
        pthread_mutex_destroy(&mutex[i]);
    }

    // destroy semaphore objects
    for (int i = 0; i < num_sem; i++)
    {
        sem_destroy(&semaphore[i]);
    }

    munmap(ptr, length); // unmap the memory

    shm_unlink(name); // unlink the shared memory
}

/*  
  Group G
  Author Name: Cosette Byte
  Email: cosette.byte@okstate.edu
  Date: 4/3/2025

  Program Description: Shared memory structure for holding
  a mutex and a semaphore, to be used across processes.
*/

#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <vector> 

#include <pthread.h>
#include <semaphore.h>

typedef struct{ 

}

typedef struct {
  int num_mutex;
  int num_sem;
  int sem_values[];
} shared_mem_t;



class shared_Mem { 
public:
    const char *name = "sharedMemory"; // Name for the shared memory object
    void* mem_setup(int num_mutex, int num_sem,  const int sem_values[]);
    void mem_close(void* ptr);
};

#endif

/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains methods to implement semaphore 
    and mutex locks.
*/

#ifndef SYNC_H
#define SYNC_H

#include <string>
#include <vector>
#include <unordered_map>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#include "Resource_Allocation.h"
#include "shared_Mem.h"

using namespace std;

Intersection* findIntersectionbyID(const char* intersectionID, Intersection *inter_ptr, int num_intersections);

string checkIntersectionType(const char* intersectionID, Intersection *inter_ptr, int num_intersections);

bool checkIntersectionFull(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, int *held);

bool checkIntersectionLockbyTrain(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, const char* trainID, int *held);

bool addtoWaitMatrix(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, int trainID, int *waiting);

bool lockIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held, int *waiting);

bool releaseIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held);

#endif
/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains methods to implement semaphore 
    and mutex locks.
*/

#ifndef SYNC_H
#define SYNC_H

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#include "Resource_Allocation.h"
#include "shared_Mem.h"



Intersection* findIntersectionbyID(string& intersectionID, Intersection *inter_ptr, int num_intersections);

string checkIntersectionType(string& intersectionID, Intersection *inter_ptr, int num_intersections);

bool checkIntersectionLock(shared_mem_t *shm, Intersection *inter_ptr, string& intersectionID, int trainID, int *held);

bool lockIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, string& intersectionID, int trainID, int *held);

bool releaseIntersection(sshared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, string& intersectionID, int trainID, int *held);

#endif
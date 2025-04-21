/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains methods to implement semaphore 
    and mutex locks.
*/
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#include "Resource_Allocation.h"
#include "shared_Mem.h"
#include "sync.h"

#include <iostream>

using namespace std;

/*
* findIntersectionbyID searches shared memory for a specific intersection based on its ID
* input: intersection ID reference, intersection pointer, and number of intersections
* output: returns a pointer to the intersection if found, otherwise nullptr
*/
Intersection* findIntersectionbyID(const char* intersectionID, Intersection *inter_ptr, int num_intersections){
    
    for(int i = 0; i < num_intersections; i++){
        if(strcmp(inter_ptr[i].name, intersectionID) == 0){
           return &inter_ptr[i];
        }
    }
    return nullptr; // intersection not found
}

/* 
* checkIntersectionType checks if the intersection is a mutex or semaphore
* input: intersection ID reference, intersection pointer, and number of intersections
* output: returns the type of intersection (semaphore or mutex)
*/
string checkIntersectionType(const char* intersectionID, Intersection *inter_ptr, int num_intersections){
    string type = "";
    // check if intersection is a mutex or semaphore
    Intersection *ptr = findIntersectionbyID(intersectionID, inter_ptr, num_intersections);
    if(ptr != nullptr){ // check if intersection is found
        type = ptr->type;
    }
    else{
        cerr << "checkIntersectionType [ERROR]: Intersection type not found for " << intersectionID << endl;
    }
    return type;
}

/*
* intersectionOpen takes intersection ID reference as input performs 
* checks to see if intersection is open without changing lock status
* returns true if intersection is open
*/
bool checkIntersectionFull(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, int *held){
    bool full = false;

    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    // check if intersection is locked in shared memory
    
    int max = intersection->capacity;

    for(int train = 0; train < shm->num_trains; train++){
        if(held[train * shm->num_intersections + intersection->index] == 1){
            max--;
        }

        if(max == 0){
            full = true;
            break;
        }
        else if(max > 0){
            full = false;
        }
    }

    return full;
}

/*
* checkIntersectionLockbyTrain checks if the intersection is locked by a specific train
* input: shared memory pointer, intersection pointer, intersection ID, train ID, and held matrix pointer
* returns true if the intersection is locked by the train
*/
bool checkIntersectionLockbyTrain(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, const char* trainID, int *held){
    bool locked = false;
    int trainIDNum = stoi(string(trainID).substr(5)); // convert string to integer
    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    
    // check if intersection is locked in shared memory
    int held_num = held[trainIDNum * shm->num_intersections + intersection->index];
    if(held_num == 1){
        // if the intersection is 1 in the held matrix it is locked
        locked = true;
    }
    
    else if(held_num == 0){
        // the intersection is not held by this train
        locked = false;
    }
    else{ 
        cerr << "[ERROR]: Held matrix error at " << trainID << " " << intersectionID << "\nHeld Num: " << held_num << endl;
    }

    return locked;
}

/*
* LockIntersection locks an intersection based on the type of lock
* (semaphore or mutex) and adds the train ID to the held matrix
* input: shared memory pointer, intersection pointer, semaphore pointer, mutex pointer,
* intersection ID, train ID, and held matrix pointer
* returns true if lock was able to be acquired
*/
bool lockIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held){
    bool locked = false; // set default to false to protect from errors
    int trainIDNum = stoi(string(trainID).substr(5)); // convert string to integer

    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    locked = (checkIntersectionLockbyTrain(shm, inter_ptr, intersectionID, trainID, held) || checkIntersectionFull(shm, inter_ptr, intersectionID, held)); // check if intersection is locked or full
    
    // check if intersection is locked (the intersection is full)
    if(locked){
        cerr << "lockIntersection: intersection is already locked or full" << endl;
        // does this need to help with wait queue?
    }

    // if intersection is unlocked check the type
    else{
        string type = checkIntersectionType(intersectionID, inter_ptr, shm->num_intersections);
        // lock the intersection based on the lock type

        if(type == "Semaphore"){
            // lock semaphore
            sem_wait(&sem[intersection->sem_index]);

            // add train ID to intersection in resource allocation table
            held[trainIDNum * shm->num_intersections + intersection->index] = 1; // set held matrix to 1
            locked = true;
        }

        else if(type == "Mutex"){
            // lock mutex
            pthread_mutex_lock(&mutex[intersection->mutex_index]);

            // add train ID to intersection in resource allocation table
            held[trainIDNum * shm->num_intersections + intersection->index] = 1; // set held matrix to 1
            locked = true;
        }

        else { // if intersection is invalid throw error
            cerr << "lockIntersection [ERROR]: " << intersectionID << " invalid intersection type." << endl;
        }

    }

    return locked;
}


/*
* releaseIntersection takes intersection ID as input
* unlocks semaphore or mutex
* returns if lock was able to be released.
*/
bool releaseIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held){
    int trainIDNum = stoi(string(trainID).substr(5)); // convert string to integer for held matrix

    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    bool released = false; // default to false to decrease errors

    if(checkIntersectionLockbyTrain(shm, inter_ptr, intersectionID, trainID, held)){
        released = true; // set release flag to true
        string type = checkIntersectionType(intersectionID, inter_ptr, shm->num_intersections);
        
        // lock the intersection based on the intersection/lock type
        if(type == "Semaphore"){
            // lock semaphore
            sem_post(&sem[intersection->sem_index]);
        }

        else if(type == "Mutex"){
            // lock mutex
            pthread_mutex_unlock(&mutex[intersection->mutex_index]);
        }
        else { // if intersection is invalid throw error
            cerr << "releaseIntersection [ERROR]: " << intersectionID << "invalid intersection type." << endl;
        }

        // remove train ID from intersection in resource allocation table
        held[trainIDNum * shm->num_intersections + intersection->index] = 0; // set held matrix to 0
    }
    
    // check intersection type

    return released;
}


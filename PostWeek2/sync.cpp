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

// findIntersectionbyID searches shared memory for a specific intersection based on its ID
// input: intersection ID reference, intersection pointer, and number of intersections
// output: returns a pointer to the intersection if found, otherwise nullptr
Intersection* findIntersectionbyID(const char* intersectionID, Intersection *inter_ptr, int num_intersections){
    
    for(int i = 0; i < num_intersections; i++){
        if(strcmp(inter_ptr[i].name, intersectionID) == 0){
           return &inter_ptr[i];
        }
    }
    return nullptr; // intersection not found
}

// checkIntersectionType checks if the intersection is a mutex or semaphore
// input: intersection ID reference, intersection pointer, and number of intersections
// output: returns the type of intersection (semaphore or mutex)
string checkIntersectionType(const char* intersectionID, Intersection *inter_ptr, int num_intersections){
    string type = "";
    // check if intersection is a mutex or semaphore
    Intersection *ptr = findIntersectionbyID(intersectionID, inter_ptr, num_intersections);
    if(ptr != nullptr){ // check if intersection is found
        type = ptr->type;
    }
    else{
        cerr << "Intersection not found" << endl;
    }
    return type;
}

// intersectionOpen takes intersection ID reference as input performs 
// checks to see if intersection is open without changing lock status
// returns true if intersection is open
bool checkIntersectionFull(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, int *held){
    bool unlocked = true;

    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    // check if intersection is locked in shared memory
    
    int max = intersection->capacity;

    for(int train = 0; train < shm->num_trains; train++){
        if(held[train * shm->num_intersections + intersection->index] == 1){
            max--;
        }

        if(max == 0){
            unlocked = false;
            break;
        }
        else if(max > 0){
            unlocked = true;
        }
    }

    return unlocked;
}

// checkIntersectionLockbyTrain checks if the intersection is locked by a specific train
// input: shared memory pointer, intersection pointer, intersection ID, train ID, and held matrix pointer
// returns true if the intersection is locked by the train
bool checkIntersectionLockbyTrain(shared_mem_t *shm, Intersection *inter_ptr, const char* intersectionID, const char* trainID, int *held){
    bool locked = false;
    int trainIDNum = std::stoi(std::string(trainID).substr(5)); // convert string to integer
    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    // check if intersection is locked in shared memory
    
    if(held[trainIDNum * shm->num_intersections + intersection->index] == 1){
        locked = true;
    }
    else{
        locked = false;
    }

    return locked;
}

// LockIntersection locks an intersection based on the type of lock
// (semaphore or mutex) and adds the train ID to the held matrix
// input: shared memory pointer, intersection pointer, semaphore pointer, mutex pointer,
// intersection ID, train ID, and held matrix pointer
// returns true if lock was able to be acquired
bool lockIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held){
    bool locked = false;
    int trainIDNum = std::stoi(std::string(trainID).substr(5)); // convert string to integer

    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    locked = checkIntersectionFull(shm, inter_ptr, intersectionID, held);
    
    // check if intersection is locked
    if(locked){
        std::cerr << "intersection is already locked" << std::endl;
        return locked;
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

        else{ // if the intersection type is invalid return an error
            std::cerr << "Invalid intersection type" << std::endl;
            return locked;
        }

    }

    return locked;
}


// releaseIntersection takes intersection ID as input
// unlocks semaphore or mutex
// returns if lock was able to be released.

bool releaseIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, const char* intersectionID, const char* trainID, int *held){
    int trainIDNum = std::stoi(std::string(trainID).substr(5)); // convert string to integer    bool released = false;
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    bool released = false;

    if(!checkIntersectionLockbyTrain(shm, inter_ptr, intersectionID, trainID, held)){
        released = true; // set release flag to true
        string type = checkIntersectionType(intersectionID, inter_ptr, shm->num_intersections);
        // lock the intersection based on the lock type

        if(type == "Semaphore"){
            // lock semaphore
            sem_post(&sem[intersection->sem_index]);
        }

        else if(type == "Mutex"){
            // lock mutex
            pthread_mutex_unlock(&mutex[intersection->mutex_index]);
        }
        // remove train ID from intersection in resource allocation table
        held[trainIDNum * shm->num_intersections + intersection->index] = 0; // set held matrix to 0
    }
    
    // check intersection type

    return released;
}


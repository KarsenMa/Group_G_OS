/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains methods to implement semaphore 
    and mutex locks.
*/

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>

#include "Resource_Allocation.h"
#include "shared_Mem.h"

#include "sync.h"

Intersection* findIntersectionbyID(string& intersectionID, Intersection *inter_ptr, int num_intersections){
    
    for(int i = 0; i < num_intersections; i++){
        if(std::strcmp(inter_ptr[i].name, id), sizeof(inter_ptr[i].name) == 0){
           return &inter_ptr[i];
        }
    }
    return nullptr; // intersection not found
}

string checkIntersectionType(string& intersectionID, Intersection *inter_ptr, int num_intersections){
    string type = "";
    // check if intersection is a mutex or semaphore
    Intersection *ptr = findIntersection(intersectionID, inter_ptr, num_intersections);
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
bool checkIntersectionFull(shared_mem_t *shm, Intersection *inter_ptr, string& intersectionID, int *held){
    bool unlocked = true;

    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    // check if intersection is locked in shared memory
    
    int max = intersection.capacity;

    for(int train = 0; train < shm->num_trains; train++){
        if(held[train * shm->num_intersections + intersection.index] == 1){
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


bool checkIntersectionLockbyTrain(shared_mem_t *shm, Intersection *inter_ptr, string& intersectionID, int trainID, int *held){
    bool locked = false;

    // get intersection in shared memory
    Intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    // check if intersection is locked in shared memory
    
    if(held[trainID * shm->num_intersections + intersection->index] == 1){
        locked = true;
    }
    else{
        locked = false;
    }

    return locked;
}

bool lockIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, string& intersectionID, int trainID, int *held){
    bool locked = false;

    intersection *intersection = findIntersectionbyID(intersectionID, inter_ptr, shm->num_intersections);
    locked = checkIntersectionLock(shm, inter_ptr, intersectionID, trainID, held);
    
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
            sem_wait(sem[intersection->sem_index]);

            // add train ID to intersection in resource allocation table
            held[trainID * shm->num_intersections + intersection->index] = 1; // set held matrix to 1
            locked = true;
        }

        else if(type == "Mutex"){
            // lock mutex
            pthread_mutex_lock(&mutex[intersection->mutex_index]);

            // add train ID to intersection in resource allocation table
            held[trainID * shm->num_intersections + intersection->index] = 1; // set held matrix to 1
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

bool releaseIntersection(shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, string& intersectionID, int trainID, int *held){

    bool released = false;

    if(!checkIntersectionLockbyTrain(shm, inter_ptr, intersectionID, trainID, held)){
        released = true; // set release flag to true
        
        // remove train ID from intersection in resource allocation table
        held[trainID * shm->num_intersections + intersection->index] = 0; // set held matrix to 0
    }
    
    // check intersection type

    return released;
}


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
#include <sync.h>

// intersectionOpen takes intersection ID reference as input performs 
// checks to see if intersection is open without changing lock status
bool checkIntersectionLock(string& intersectionID, string& trainID){
    bool unlocked = false;
    // check if intersection is locked in shared memory
    

    // check if train already exists in intersection


    // this will need to integrate with the resource allocation table in order to work.
    // (semaphore index needs to be given to specific intersection)

    

    return unlocked;
}

bool lockIntersection(string& intersectionID, string& trainID){
    bool locked = false;
    
    // check intersection type
    // if semaphore, lock semaphore
    // if mutex, lock mutex
    // add train to intersection in resource allocation table

    return locked;
}


// releaseIntersection takes intersection ID as input
// unlocks semaphore or mutex
// returns if lock was able to be released.

bool releaseIntersection(string& intersectionID, string& trainID){

    bool released = false;

    if(!checkIntersectionLock(intersectionID, trainID)){
        released = true;

        // remove train ID from intersection in resource allocation table

        return released;
    }
    
    // check intersection type

    return released;
}


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
// lock if intersection is open, returns true if intersection was able to be locked
bool intersectionOpen(string& intersectionID){
    bool lockAcquired = false;
    // check if intersection is locked in shared memory

    // this will need to integrate with the resource allocation table in order to work.
    // (semaphore index needs to be given to specific intersection)

    // update semaphore or mutex lock based on intersection ID 

    return lockAcquired;

    
}


// releaseIntersection takes intersection ID as input
// unlocks semaphore or mutex
// returns if lock was able to be released.

bool releaseIntersection(string& intersection_id){
    bool released = false;
    
    // check intersection type

    return released;
}


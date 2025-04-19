/*  
    Group G
    Author Name: Eric Vo
    Email: eric.t.vo@okstate.edu
    Date: 17 April 2025

    Program Description: When a deadlock is detected, intersections and trains are sent here to be forcibly released and resolve the deadlock.
*/

// required headers
#include <iostream>
#include <cstring>

#include "DeadlockResolution.h"
#include "sync.h"
#include "TrainCommunication.h"

using namespace std;

/*
*   function: resolvedDeadlock is called from detectAndResolveDeadlock
*             it is used when a deadlock is detected to forcibly release a held intersection when from a train
*
*   input: shared_mem_t shm points to a shared memory block that stores held intersections from a train that is selected
*          
*          intersections is a vector of all intersections parsed into the system; it is used to map names for releasing 
*          
*          trainToPrempt is the ID of a train that is selected to break the deadlock
*          
*          intersectionToRelease is the name of the intersection that is being held by a train that will be released forcibly
*
*/
void resolveDeadlock(shared_mem_t* shm, const vector<Intersection>& intersections, const char* trainToPreempt, const char* intersectionToRelease) {
    
    // accesses the shared memory layout
    char* base = reinterpret_cast<char*>(shm) + sizeof(shared_mem_t);
    int* sem_val_block = reinterpret_cast<int*>(base);
    pthread_mutex_t* mutex = reinterpret_cast<pthread_mutex_t*>(sem_val_block + shm->num_sem);
    sem_t* sem = reinterpret_cast<sem_t*>(mutex + shm->num_mutex);
    Intersection* inter_ptr = reinterpret_cast<Intersection*>(sem + shm->num_sem);
    int* held = reinterpret_cast<int*>(inter_ptr + shm->num_intersections);

    // this logs the action in the console when taken
    cout << "The server detected a deadlock involving " << trainToPreempt << " holding " << intersectionToRelease << ".\n";
    cout << "Forcibly releasing " << intersectionToRelease << " from " << trainToPreempt << ".\n";

    // performs the release
    releaseIntersection(shm, inter_ptr, sem, mutex, intersectionToRelease, trainToPreempt, held);

    // confirms in console and logs the release in simulation.log
    cout << "Cycle is broken. Trains may proceed.\n";
    logMessage(string(trainToPreempt) + " released " + intersectionToRelease + " forcibly to resolve deadlock.");
}

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unordered_map>
#include <vector>


#include "MutexThread.h"
#include "IPCSemaphore.h"
#include "shared_Mem.h"
#include "sync.h"

int main(){

    // Parse intersections and trains files into usable format
    unordered_map<string, vector<string>> intersections;
    unordered_map<string, vector<string>> trains;

    parseFile("/data/intersections.txt", intersections);
    parseFile("/data/intersections.txt", trains);

    // TO DO: create resource allocation table


    // TO DO: count types of intersections from parsed file
    int num_mutex;
    int num_sem;



    // create shared memory using number of intersections to set size of shared memory
    NUM_MUTEXES = num_mutex;
    NUM_SEMS = num_sem;
    shared_Mem mem;
    void *ptr = mem.mem_setup();
    shared_mem_t* m = (shared_mem_t*)ptr;

    // setup message queues
    setupMessageQueues()


    // TO DO: implement synchronization




}

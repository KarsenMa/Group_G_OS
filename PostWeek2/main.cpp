/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains the main function which integrates all the other portions in order to synchronize
    the processes.
    */ 

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unordered_map>
#include <vector>


#include "MutexThread.h"
#include "IPCSemaphore.h"
#include "shared_Mem.h"
#include "sync.h"

// This function performs the child process functions
// input is the train and its route
void childProcess(string train, vector<string> route, int requestQueue, int responseQueue){
    // childProcess takes path and train information
    // childProcess will use message queue to acquire and release semaphore and mutex locks

    simulateTrainMovement(&train, &route, requestQueue, responseQueue);


}


int main(){

    // Parse intersections and trains files into usable format
    unordered_map<string, vector<string>> intersections;
    unordered_map<string, vector<string>> trains;

    parseFile("/data/intersections.txt", intersections);
    parseFile("/data/intersections.txt", trains);


    std::ofstream logFile;  // For logging
    int simulatedTime = 0;  // For simulated time tracking

    // TO DO: create resource allocation table


    // count types of intersections from parsed file or from resource table
    int num_mutex = 0;
    int num_sem = 0;

    for(auto iter = intersections.begin(); iter != intersections.end(); ++iter) {
        int currentValue = std::stoi(iter->second[0]); // convert string to integer
        if(currentValue > 1){ 
            num_sem++;
        }
        else if(currentValue == 1){
            num_mutex++;
        }
        else{
            cerr << "invalid intersection capacity" << endl;
        }
    }
    

    // create shared memory using number of intersections to set size of shared memory
    shared_Mem mem;

    // use calculated number of intersections for mutex and semaphore to provide size for shared memory
    void *ptr = mem.mem_setup(num_mutex, num_sem);
    shared_mem_t* m = (shared_mem_t*)ptr;

    // setup message queues
    int requestQueue = 0;
    int responseQueue = 0;
    if(setupMessageQueues(requestQueue, responseQueue) == -1){
        std::cerr << "Could not set up message queues.\n";
        return; 
    }
    


    // TO DO: Create child processes, (call child process function inside forking function)
    
    
    // TO DO: implement server side message queue stuff
    if(getpid != 0) { // if the process is the parent process, run the server side
        processTrainRequests(requestQueue, responseQueue);
    } 


    // TO DO: implement synchronization




}

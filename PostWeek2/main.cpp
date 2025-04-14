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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shared_Mem.h"
#include "sync.h"
#include "Resource_Allocation.h"


// This function performs the child process functions
// input is the train and its route
void childProcess(string train, vector<string> route, int requestQueue, int responseQueue){
    // childProcess takes path and train information
    // childProcess will use message queue to acquire and release semaphore and mutex locks

    simulateTrainMovement(&train, &route, requestQueue, responseQueue);
}

vector<pid_t> forkTrains(unordered_map<string, vector<string>> trains, int requestQueue, int responseQueue){
    vector<pid_t> childPIDS;
    for(auto iter : trains) { 
        pid_t pid = fork();

        if(pid == -1) {
            cerr << "Fork failed" << endl;
            exit(1);
        }
        else if(pid == 0) { // Child process
            childProcess(iter.first, iter.second, requestQueue, responseQueue);
            exit(0); // Child process exits after running
        }
        else { // Parent process
            childPIDS.push_back(pid); // Store child PID
        }
    } 
    return childPIDS;  
}



int main(){
    pid_t serverPID = getpid(); // get server process ID 

    // Parse intersections and trains files into usable format
    vector<Intersection> intersections;
    unordered_map<string, vector<string>> trains;

    parseIntersections("/data/intersections.txt", intersections);
    parseFile("/data/intersections.txt", trains);


    std::ofstream logFile;  // For logging
    int simulatedTime = 0;  // For simulated time tracking

    // initializes the log file and opens the "simulation.log" file for logMessage in TrainCommunication.cpp to write in
    // simulation.log must be open for TrainCommunication.cpp to be able to write in
    logFile.open("data/simulation.log", std::ios::out);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open simulation.log for writing." << std::endl;
        return -1;
    }

    // TO DO: create resource allocation table


    // count types of intersections from parsed file or from resource table
    int num_mutex = 0;
    int num_sem = 0;
    int num_trains = trains.size(); // number of trains
    int sem_values[intersections.size()]; // array to hold semaphore values

    for(auto iter = intersections.begin(); iter != intersections.end(); ++iter) {
        int currentValue = iter->capacity; // convert string to integer
        if(currentValue > 1){ 
            num_sem++;
            sem_values[num_sem - 1] = currentValue; // store semaphore value
        }
        else if(currentValue == 1){
            num_mutex++;
        }
        else{
            cerr << "invalid intersection capacity" << endl;
        }
    }
    
    // logs to simulation.log when the system is first initalized
    logFile << "[00:00:00] SERVER: Initialized intersections:\n";

    // create shared memory using number of intersections to set size of shared memory
    shared_Mem mem;

    // use calculated number of intersections for mutex and semaphore to provide size for shared memory
    void *ptr = mem.mem_setup(num_mutex, num_sem, sem_values, num_trains);
    shared_mem_t* shm_ptr = (shared_mem_t*)ptr;
    
    char *base = reinterpret_cast<char *>(ptr) + sizeof(shared_mem_t);
    
    int *sem_val_block = reinterpret_cast<int *>(base);
    
    pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(sem_val_block + num_sem);
    sem_t *semaphore = reinterpret_cast<sem_t *>(mutex + num_mutex);
    // setup pointers to Intersection structs
    Intersection *inter_ptr = reinterpret_cast<Intersection *>(
        reinterpret_cast<char *>(semaphore) + num_sem * sizeof(sem_t));
    // set pointer to *held matrix
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(inter_ptr) + (num_sem + num_mutex) * sizeof(Intersection));

    // TO DO: setup resource allocation table


    // setup message queues
    int requestQueue = 0;
    int responseQueue = 0;
    if(setupMessageQueues(requestQueue, responseQueue) == -1){
        std::cerr << "Could not set up message queues.\n";
        return -1; 
    }
    
    // TO DO: create resource allocation graph

    detectAndResolveDeadlock(ptr, intersections); // pass in shared memory pointer and vector of intersections

    // create child processes for each train and store their PIDs
    vector<pid_t> childPIDS = forkTrains(trains, requestQueue, responseQueue); // fork the number of trains


    if(getpid() == serverPID) { // if the process is the parent process, run the server side

        processTrainRequests(requestQueue, responseQueue);
        for (auto &pid : childPIDS) { // wait for the child processes to finish
            waitpid(pid, nullptr, 0);
        }
        std::cout << "All trains have finished." << std::endl;
        logFile << "All trains have finished." << std::endl;
    } 
    
    // cleanup message queues
    cleanupMessageQueues(requestQueue, responseQueue);


    if (logFile.is_open()) { // cleanup log file
        logFile.close();
    }

    mem_close(ptr); // cleanup shared memory

    std::cout << "All processes finished." << std::endl;
    logFile << "All processes finished." << std::endl;
    return 0;
}

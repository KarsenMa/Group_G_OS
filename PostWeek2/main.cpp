/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains the main function for the train simulation.
    It uses message queues to communicate between the server and child processes.
    The server process forks child processes for each train and uses shared memory
    to store the state of the intersections and trains. 

    g++ -o railway DeadlockDetection.cpp main.cpp Resource_Allocation.cpp shared_Mem.cpp sync.cpp TrainCommunication.cpp -lpthread

    */ 

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iomanip>  // For setw
#include <sstream>  // For stringstream
#include <cstring>  

#include "shared_Mem.h"
#include "sync.h"
#include "Resource_Allocation.h"
#include "TrainCommunication.h"
#include "DeadlockDetection.h"

using namespace std;

#include <algorithm>


void printTrainsAndAttributes(
    const unordered_map<string, vector<string>> &trains,
    const Intersection *inter_ptr,
    size_t num_intersections)
{
    cout << "Train Routes and Intersection Attributes:\n";
    cout << string(60, '=') << endl;

    for (const auto &train : trains) {
        const string &trainName = train.first;
        const vector<string> &route = train.second;

        cout << "Train: " << trainName << endl;
        if (route.empty()) {
            cout << "  Route: [empty]\n";
            continue;
        }

        for (const string &intersectionName : route) {
            bool found = false;

            for (size_t i = 0; i < num_intersections; ++i) {
                if (strcmp(inter_ptr[i].name, intersectionName.c_str()) == 0) {
                    const Intersection &inter = inter_ptr[i];
                    cout << "  - Intersection: " << inter.name << "\n"
                         << "    Type       : " << inter.type << "\n"
                         << "    Index      : " << inter.index << "\n"
                         << "    Capacity   : " << inter.capacity << "\n"
                         << "    " << (strcmp(inter.type, "Semaphore") == 0 ?
                                    ("Semaphore Index: " + to_string(inter.sem_index)) :
                                    ("Mutex Index    : " + to_string(inter.mutex_index)))
                         << "\n";
                    found = true;
                    break;
                }
            }

            if (!found) {
                cout << "  - Intersection: " << intersectionName << " [NOT FOUND]\n";
            }
        }

        cout << string(60, '-') << endl;
    }
}



// This function performs the child process functions
// input: train name
// input: vector of strings for the route
// input: requestQueue and responseQueue for message queue
void childProcess(const char* train, vector<string> route, int requestQueue, int responseQueue, 
                  shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *semaphore, pthread_mutex_t *mutex) {
    // childProcess takes path and train information
    // childProcess will use message queue to acquire and release semaphore and mutex locks
    simulateTrainMovement(train, route, requestQueue, responseQueue, shm, inter_ptr, held, semaphore, mutex); // simulate train movement
}

// This function forks the child processes for each train
// input: unordered map of trains and their routes
// input: requestQueue and responseQueue for message queue
// output: vector of child PIDs
vector<pid_t> forkTrains(unordered_map<string, vector<string>> trains, int requestQueue, int responseQueue, 
                          shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *semaphore, pthread_mutex_t *mutex) {
    vector<pid_t> childPIDS;
    for(auto iter : trains) { 
        pid_t pid = fork();
        //cout << "Forking process for train: " << iter.first << "\nPID: " << getpid() << endl;
        if(pid == -1) {
            cerr << "Fork failed" << endl;
            perror("fork");
            exit(1);
        }
        else if(pid == 0) { // Child process
            childProcess(iter.first.c_str(), iter.second, requestQueue, responseQueue, 
                         shm, inter_ptr, held, semaphore, mutex);
            exit(0); // Child process exits after running
        }
        else { // Parent process
            childPIDS.push_back(pid); // Store child PID
        }
    } 
    return childPIDS;  
}

// From Resource Allocation
// Parse trains.txt to map train IDs to their routes
void parseTrains(const string &filename, unordered_map<string, vector<string>> &trains) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open trains file: " << filename << endl;
        return;
    }
    string line;
    
    while (getline(file, line)) {
        size_t colon = line.find(':');
        if (colon == string::npos)
            continue;

        string trainName = line.substr(0, colon); // e.g., Train0
        string routeData = line.substr(colon + 1);

        stringstream ss(routeData);
        string intersection;
        vector<string> route;
        
        while (getline(ss, intersection, ',')) {
            route.push_back(intersection);
        }
        
        trains[trainName] = route;
    }
}

// Display the resource allocation table from shared memory
void printIntersectionStatus(shared_mem_t *shm, Intersection *inter_ptr, int *held) {
    cout << left << setw(15) << "IntersectionID"
         << setw(10) << "Type"
         << setw(10) << "Capacity"
         << setw(12) << "Lock State"
         << "Holding Trains" << endl;

    cout << string(60, '-') << endl;

    for (int i = 0; i < shm->num_intersections; ++i) {
        string lockState = "Unlocked";

        for (int t = 0; t < shm->num_trains; ++t) {
            if (held[t * shm->num_intersections + i] == 1) {
                lockState = "Locked";
                break;
            }
        }

        cout << left << setw(15) << inter_ptr[i].name
             << setw(10) << inter_ptr[i].type
             << setw(10) << inter_ptr[i].capacity
             << setw(12) << lockState
             << "[";

        bool first = true;
        for (int t = 0; t < shm->num_trains; ++t) {
            if (held[t * shm->num_intersections + i] == 1) {
                if (!first)
                    cout << ", ";
                cout << "Train" << t;
                first = false;
            }
        }

        cout << "]" << endl;
    }
}

// These should be global variables:
std::ofstream logFile;  // For logging
int simulatedTime = 0;  // For simulated time tracking

int main(){
    pid_t serverPID = getpid(); // get server process ID 

    // Parse intersections and trains files into usable format
    vector<Intersection> intersections;
    unordered_map<string, vector<string>> trains;

    parseIntersections("data/intersections.txt", intersections);
    parseTrains("data/trains.txt", trains); // Replace commented-out parseFile line

    
    // initializes the log file and opens the "simulation.log" file for logMessage in TrainCommunication.cpp to write in
    // simulation.log must be open for TrainCommunication.cpp to be able to write in
    logFile.open("data/simulation.log", std::ios::out);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open simulation.log for writing." << std::endl;
        return -1;
    }


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

    // setup intersection data in shared memory
    int count_sem = 0;
    int count_mutex = 0;
        int j = 0;
    for(size_t i = 0; i < intersections.size(); ++i){ 
        
        inter_ptr[i] = intersections[i]; // copy intersection data into shared memory
        inter_ptr[i].index = j; // set index for each intersection
        if(strcmp(inter_ptr[i].type, "Semaphore") == 0){
            inter_ptr[i].sem_index = count_sem; // set semaphore index
            count_sem++;
            }
        else if(strcmp(inter_ptr[i].type, "Mutex") == 0){
            inter_ptr[i].mutex_index = count_mutex; // set mutex index
            count_mutex++;
        }
        else{
            std::cerr << "Invalid intersection type" << inter_ptr[i].type << std::endl;
        }
        j++;
    }

    printTrainsAndAttributes(trains, inter_ptr, intersections.size()); // Print train routes and intersection attributes


    // setup message queues
    int requestQueue = 0;
    int responseQueue = 0;
    if(setupMessageQueues(requestQueue, responseQueue) == -1){
        std::cerr << "Could not set up message queues.\n";
        return -1; 
    }
    
    // Create resource allocation graph
    printIntersectionStatus(shm_ptr, inter_ptr, held); // Display resource allocation table

    detectAndResolveDeadlock(static_cast<shared_mem_t*>(ptr), intersections); // pass in shared memory pointer and vector of intersections

    // create child processes for each train and store their PIDs
    vector<pid_t> childPIDS = forkTrains(trains, requestQueue, responseQueue, shm_ptr, inter_ptr, held, semaphore, mutex); // fork the number of trains

    if(getpid() == serverPID) { // if the process is the parent process, run the server side

        processTrainRequests(requestQueue, responseQueue, shm_ptr, inter_ptr, held, semaphore, mutex); // process train requests
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

    mem.mem_close(ptr); // cleanup shared memory

    std::cout << "All processes finished." << std::endl;
    logFile << "All processes finished." << std::endl;
    return 0;
}

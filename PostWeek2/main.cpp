/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/9/2025
    Program Description: This file contains the main function for the train simulation.
    It uses message queues to communicate between the server and child processes.
    The server process forks child processes for each train and uses shared memory
    to store the state of the intersections and trains.

    g++ -o railway DeadlockDetection.cpp main.cpp Resource_Allocation.cpp shared_Mem.cpp sync.cpp TrainCommunication.cpp DeadlockResolution.cpp -lpthread

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
#include <iomanip> // For setw
#include <sstream> // For stringstream
#include <cstring>

#include "shared_Mem.h"
#include "sync.h"
#include "Resource_Allocation.h"
#include "TrainCommunication.h"
#include "DeadlockDetection.h"

using namespace std;

#include <algorithm>

/* This function performs the child process functions
 *  input: train name
 *  input: vector of strings for the route
 *  input: requestQueue and responseQueue for message queue
 */
void child_process(const char *train, vector<string> route, int requestQueue, int responseQueue,
                   shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *semaphore, pthread_mutex_t *mutex)
{
    // child_process takes path and train information
    // child_process will use message queue to acquire and release semaphore and mutex locks
    simulateTrainMovement(train, route, requestQueue, responseQueue, shm, inter_ptr, held, semaphore, mutex); // simulate train movement
}

/* This function forks the child processes for each train
 *  input: unordered map of trains and their routes
 *  input: requestQueue and responseQueue for message queue
 *  output: vector of child PIDs
 */
vector<pid_t> forkTrains(unordered_map<string, vector<string>> trains, int requestQueue, int responseQueue,
                         shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *semaphore, pthread_mutex_t *mutex)
{
    vector<pid_t> childPIDS;
    for (auto iter : trains)
    {
        pid_t pid = fork();
        // cout << "Forking process for train: " << iter.first << "\nPID: " << getpid() << endl;
        if (pid == -1)
        {
            cerr << "forkTrains [ERROR]: Fork failed" << endl;
            perror("fork");
            exit(1);
        }
        else if (pid == 0)
        { // Child process
            char trainID[16];
            strncpy(trainID, iter.first.c_str(), sizeof(trainID) - 1);
            trainID[sizeof(trainID) - 1] = '\0'; // null termination to reduce junk errors

            // run the child process in the fork
            child_process(trainID, iter.second, requestQueue, responseQueue,
                          shm, inter_ptr, held, semaphore, mutex);
            exit(0); // Child process exits after running
        }
        else
        {                             // Parent process
            childPIDS.push_back(pid); // Store child PID
        }
    }
    return childPIDS; // return child PIDs for use in main
}

/* From Resource Allocation */
/* Parse trains.txt to map train ids to routes */
void parseTrains(const string &filename, unordered_map<string, vector<string>> &trains)
{
    ifstream file(filename); /*open trains.txt*/
    if (!file.is_open())
    {
        cerr << "parseTrains [ERROR]: Failed to open" << filename << endl; /*error detection*/
        return;
    }
    string line;

    while (getline(file, line)) /*Read trains.txt*/
    {
        size_t colon = line.find(':'); /*colon that breaks train id from route*/
        if (colon == string::npos)     /*if No colon, skip line invalid format*/
            continue;

        string trainName = line.substr(0, colon);  /*before colon is train id*/
        string routeData = line.substr(colon + 1); /*after colon is the route*/

        stringstream ss(routeData); /*stringstream to parse data*/
        string intersection;
        vector<string> route; /*Vector to hold route*/

        while (getline(ss, intersection, ',')) /*intersection need to be stored after each comma*/
        {
            route.push_back(intersection);
        }

        trains[trainName] = route; /*map train id to route*/
    }
}

/* From Resouce ALlocation*/
/* Print Resource ALlocation Table */
void printIntersectionStatus(shared_mem_t *shm, Intersection *inter_ptr, int *held)
{
    /*Columns for Resource Allocation Table*/
    cout << left << setw(15) << "IntersectionID"
         << setw(10) << "Type"
         << setw(10) << "Capacity"
         << setw(12) << "Lock State"
         << "Holding Trains" << endl;

    /*Seperate columns from data*/
    cout << string(60, '_') << endl;

    /*Loop through intersections*/
    for (int i = 0; i < shm->num_intersections; ++i)
    {
        /*Lock is not held*/
        string lockState = "Unlocked";

        /*Chech Lock State*/
        for (int t = 0; t < shm->num_trains; ++t)
        {
            if (held[t * shm->num_intersections + i] == 1) /*find intersections held by train*/
            {
                /*If train holds intersection set locked*/
                lockState = "Locked";
                break;
            }
        }

        /*Intersection data*/
        cout << left << setw(15) << inter_ptr[i].name /*name*/
             << setw(10) << inter_ptr[i].type         /*lock type*/
             << setw(10) << inter_ptr[i].capacity     /*inersection capacity*/
             << setw(12) << lockState                     /*Locked/Unlocked*/
             << "[";

        bool one = true;

        /*Loop for printing trains that hold lock on specific intersection*/
        for (int t = 0; t < shm->num_trains; ++t)
        {
            /*True value in held matrix*/
            if (held[t * shm->num_intersections + i] == 1)
            {
                if (!one) /*Multiple elements separate with comma*/
                    cout << ", ";
                cout << "Train" << t; /*Trains Id*/
                one = false;
            }
        }

        cout << "]" << endl;
    }
}

// These should be global variables:
ofstream logFile;      // For logging
int simulatedTime = 0; // For simulated time tracking

/* main handles server side, sets up message queues, forks child processes
 * sets up shared memory, logging.
 */
int main()
{
    pid_t serverPID = getpid(); // get server process ID

    // Parse intersections and trains files into usable format
    vector<Intersection> intersections;
    unordered_map<string, vector<string>> trains;

    parseIntersections("data/intersections.txt", intersections);
    parseTrains("data/trains.txt", trains); // Replace commented-out parseFile line

    // initializes the log file and opens the "simulation.log" file for logMessage in TrainCommunication.cpp to write in
    // simulation.log must be open for TrainCommunication.cpp to be able to write in
    logFile.open("data/simulation.log", ios::out);
    if (!logFile.is_open())
    {
        cerr << "Main [ERROR]: Failed to open simulation.log for writing." << endl;
        return -1;
    }

    // count types of intersections from parsed file or from resource table
    int num_mutex = 0;
    int num_sem = 0;
    int num_trains = trains.size();       // number of trains
    int sem_values[intersections.size()]; // array to hold semaphore values

    for (auto iter = intersections.begin(); iter != intersections.end(); ++iter)
    {
        int currentValue = iter->capacity; // convert string to integer

        if (currentValue > 1)
        { // multiple train capacity indicates semaphore intersection
            num_sem++;
            sem_values[num_sem - 1] = currentValue; // store semaphore value
        }
        else if (currentValue == 1)
        { // single train capacity indicates mutex intersection
            num_mutex++;
        }
        else
        { // error handling
            cerr << "Main [ERROR]: invalid intersection capacity for " << iter->name << endl;
        }
    }

    // logs to simulation.log when the system is first initalized

    // create shared memory using number of intersections to set size of shared memory
    shared_Mem mem;

    // use calculated number of intersections for mutex and semaphore to provide size for shared memory
    void *ptr = mem.mem_setup(num_mutex, num_sem, sem_values, num_trains);
    shared_mem_t *shm_ptr = (shared_mem_t *)ptr;

    char *mem_struct = reinterpret_cast<char *>(ptr) + sizeof(shared_mem_t);

    int *sem_val_block = reinterpret_cast<int *>(mem_struct);

    pthread_mutex_t *mutex = reinterpret_cast<pthread_mutex_t *>(sem_val_block + num_sem);
    sem_t *semaphore = reinterpret_cast<sem_t *>(mutex + num_mutex);

    // setup pointers to Intersection structs
    Intersection *inter_ptr = reinterpret_cast<Intersection *>(
        reinterpret_cast<char *>(semaphore) + num_sem * sizeof(sem_t));

    // set pointer to *held matrix
    int *held = reinterpret_cast<int *>(
        reinterpret_cast<char *>(inter_ptr) + (intersections.size()) * sizeof(Intersection));

    // setup intersection data in shared memory
    int count_sem = 0;
    int count_mutex = 0;
    int j = 0; // setup shared memory index for later use

    for (size_t i = 0; i < intersections.size(); ++i)
    {

        inter_ptr[i] = intersections[i]; // copy intersection data into shared memory
        inter_ptr[i].index = j;          // set index for each intersection
        if (strcmp(inter_ptr[i].type, "Semaphore") == 0)
        {
            inter_ptr[i].sem_index = count_sem; // set semaphore index
            count_sem++;
        }
        else if (strcmp(inter_ptr[i].type, "Mutex") == 0)
        {
            inter_ptr[i].mutex_index = count_mutex; // set mutex index
            count_mutex++;
        }
        else
        { // throw error if the intersection type is junk
            cerr << "Main [ERROR]: Invalid intersection type for " << inter_ptr[i].name << " type: " << inter_ptr[i].type << endl;
        }
        j++;
    }

    // setup message queues
    int requestQueue = 0;
    int responseQueue = 0;

    if (setupMessageQueues(requestQueue, responseQueue) == -1)
    {
        cerr << "Main [ERROR]: Could not set up message queues.\n";
        return -1;
    }

    // Create resource allocation graph
    printIntersectionStatus(shm_ptr, inter_ptr, held); // Display resource allocation table

    detectAndResolveDeadlock(static_cast<shared_mem_t *>(ptr), intersections); // pass in shared memory pointer and vector of intersections

    // create child processes for each train and store their PIDs
    vector<pid_t> childPIDS = forkTrains(trains, requestQueue, responseQueue, shm_ptr, inter_ptr, held, semaphore, mutex); // fork the number of trains

    // run the server process
    if (getpid() == serverPID)
    { // if the process is the parent process, run the server side
        logFile << "[00:00:00] SERVER: Initialized intersections:\n";
        printIntersectionStatus(shm_ptr, inter_ptr, held);

        processTrainRequests(requestQueue, responseQueue, shm_ptr, inter_ptr, held, semaphore, mutex); // process train requests
        for (auto &pid : childPIDS)
        { // wait for the child processes to finish
            waitpid(pid, nullptr, 0);
        }
        cout << "All trains have finished." << endl;
        logFile << "All trains have finished." << endl;
    }

    // close logFile is the process is a child process
    else if (getpid() != serverPID)
    {
        if (logFile.is_open())
        {
            logFile.close();
        }
    }

    // after process is finished, cleanup
    // cleanup message queues
    cleanupMessageQueues(requestQueue, responseQueue);

    logFile.close(); // close logFile

    mem.mem_close(ptr); // cleanup shared memory

    cout << "All processes finished." << endl;
    logFile << "All processes finished." << endl;
    return 0;
}

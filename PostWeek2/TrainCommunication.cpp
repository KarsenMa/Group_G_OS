//---------------------------------------- TrainCommunication.cpp ------------------------------------
/*
  Group G
  Author: Clayton Nunley, Cosette Byte, Damian Silvar
  Email: clayton.nunley@okstate.edu
  Date: April 2, 2025

  Description: This file implements the ACQUIRE/RELEASE request mechanism and basic train
  movement simulation without synchronization for the railway simulation group project.
*/

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <sys/wait.h>
#include <algorithm>
#include <sys/file.h>

#include "shared_Mem.h"
#include "TrainCommunication.h"
#include "trainCommExtension.h" // included for logging and wait queueing



// External file for logging
// extern std::ofstream logFile;

// We define this in main.cpp (so only one definition in the whole project):
extern int simulatedTime; 

// Function to get formatted timestamp
std::string getTimestamp() {
    int hours = simulatedTime / 3600;
    int minutes = (simulatedTime % 3600) / 60;
    int seconds = simulatedTime % 60;
    
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours << ":"
       << std::setfill('0') << std::setw(2) << minutes << ":"
       << std::setfill('0') << std::setw(2) << seconds;
    
    return ss.str();
}

// Function to log a message to both console and file
void logMessage(const std::string& message) {
    std::string timestamped = "[" + getTimestamp() + "] " + message + "\n";
    std::cout << timestamped;
    char fileName[] = "data/simulation.log";

    int fd = open(fileName, O_WRONLY | O_APPEND);
    if(fd == -1) { 
        std::cerr << "sendLogMessage [ERROR]: Failed to open " << fileName << " for writing." << std::endl;
        return;
    }

    if(flock(fd, LOCK_EX) == -1) {
        std::cerr << "sendLogMessage [ERROR]: Failed to lock " << fileName << "." << std::endl;
        close(fd);
        return;
    }

    write(fd, timestamped.c_str(), timestamped.size());
    
   
    //    logFile << timestamped << std::endl;
    //    logFile.flush();

    flock(fd, LOCK_UN);
    close(fd);
    return;
}


// Function to set up message queues
int setupMessageQueues(int& requestQueue, int& responseQueue, int& logQueue, int& waitQueue) {
    key_t requestKey = ftok(".", 'R');
    key_t responseKey = ftok(".", 'S');
    key_t logKey = ftok(".", 'L');
    key_t waitKey = ftok(".", 'W');
    
    requestQueue = msgget(requestKey, IPC_CREAT | 0666);
    responseQueue = msgget(responseKey, IPC_CREAT | 0666);
    logQueue = msgget(logKey, IPC_CREAT | 0666);
    waitQueue = msgget(waitKey, IPC_CREAT | 0666);
    
    if (requestQueue == -1 || responseQueue == -1 || logQueue == -1) {
        std::cerr << "Failed to create message queues: " << strerror(errno) << std::endl;
        return -1;
    }
    
    return 0;
}

// Function to clean up message queues
void cleanupMessageQueues(int requestQueue, int responseQueue, int logQueue, int waitQueue) {
    msgctl(requestQueue, IPC_RMID, nullptr);
    msgctl(responseQueue, IPC_RMID, nullptr);
    msgctl(logQueue, IPC_RMID, nullptr);
    msgctl(waitQueue, IPC_RMID, nullptr);
}

/*
* Train functions for communicating with the server
*/

// Damian
// TO DO: create function to send log messages to server (follow message send format)
bool sendLogMessage(int logQueue, const std::string& message) { 
    RequestMsg msg;
    msg.mtype = ResponseType::LOG; // Response type for logging
    strncpy(msg.train_id, message.c_str(), sizeof(msg.train_id));
    msg.intersection_id[0] = '\0'; // when no intersection needed

    if (msgsnd(logQueue, &msg, sizeof(RequestMsg) - sizeof(long), 0) == -1) {
        perror("Failed to send log message");
        return false;
    }
    return true;
}

// Function to send an ACQUIRE request
bool trainSendAcquireRequest(int requestQueue, int logQueue, const char* trainId, const char* intersectionId) {
    RequestMsg msg;


    msg.mtype = RequestType::ACQUIRE;
    strncpy(msg.train_id, trainId, sizeof(msg.train_id) - 1);
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';

    strncpy(msg.intersection_id, intersectionId, sizeof(msg.intersection_id) - 1);
    msg.intersection_id[sizeof(msg.intersection_id) - 1] = '\0';
    
    if (msgsnd(requestQueue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        std::cerr << "Failed to send ACQUIRE request: " << strerror(errno) << std::endl;
        return false;
    }
    
    sendLogMessage(logQueue, std::string(trainId) + ": Sent ACQUIRE request for " + intersectionId + ".");
    return true;
}

// Function to send a RELEASE request
bool trainSendReleaseRequestExtended(int requestQueue, int logQueue, const char* trainId, const char* intersectionId, 
    shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, int *held) {
    RequestMsg msg;
    
    msg.mtype = RequestType::RELEASE;
    strncpy(msg.train_id, trainId, sizeof(msg.train_id) - 1);
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';
    strncpy(msg.intersection_id, intersectionId, sizeof(msg.intersection_id) - 1);
    msg.intersection_id[sizeof(msg.intersection_id) - 1] = '\0';
    
    if (msgsnd(requestQueue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        std::cerr << "Failed to send RELEASE request: " << strerror(errno) << std::endl;
        return false;
    }
    else {
        // Log the release request
        // **Moved to server side** releaseIntersection(shm, inter_ptr, sem, mutex, intersectionId, trainId, held);
        sendLogMessage(logQueue, std::string(trainId) + ": Sent RELEASE request for " + intersectionId + ".");
        return true;
    }
    
    // in case of unexpected errors
    return false;
}

// Function for trains to wait for a response from the server
int trainWaitForResponse(int responseQueue, int logQueue, const char* trainId) {
    ResponseMsg msg;
    
    // For debugging:
    // std::cerr << "Received train ID: " << trainId << std::endl;
    
    // Convert train_id to a long for message filtering
    std::string tempID = std::string(trainId);
    if(tempID.size() <= 5 || !std::all_of(tempID.begin() + 5, tempID.end(), ::isdigit)) {
        std::cerr << "Invalid train ID format: " << trainId << std::endl;
        return -1;
    }
    long trainIdLong = std::stol(tempID.substr(5));
    
    // Receive response message specifically for this train
    if (msgrcv(responseQueue, &msg, sizeof(msg) - sizeof(long), trainIdLong, 0) == -1) {
        std::cerr << "Failed to receive response: " << strerror(errno) << std::endl;
        return -1;
    }

    char tempIntersectionId[20];
    strncpy(tempIntersectionId, msg.intersection_id, sizeof(msg.intersection_id) - 1);
    tempIntersectionId[sizeof(msg.intersection_id) - 1] = '\0';
    
    // Log the response received
    std::string responseTypeStr;
    switch (msg.response_type) {
        case ResponseType::GRANT:
            responseTypeStr = "GRANT";
            break;
        case ResponseType::WAIT:
            responseTypeStr = "WAIT";
            break;
        case ResponseType::DENY:
            responseTypeStr = "DENY";
            break;
        default:
            responseTypeStr = "UNKNOWN";
    }
    
    sendLogMessage(logQueue, std::string(trainId) + ": Received " + responseTypeStr + " for " + tempIntersectionId + ".");
    
    return msg.response_type;
}


// Function to simulate train movement
void simulateTrainMovement(const char* trainId, const std::vector<std::string>& route, 
                           int requestQueue, int responseQueue, int logQueue, int waitQueue, shared_mem_t *shm,
                           Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex) 
{
    // Iterate through each intersection in the route
    for (const auto& intersection : route) {
        // Request to acquire the intersection
        const char* tempIntersection = intersection.c_str();
        if (!trainSendAcquireRequest(requestQueue, logQueue, trainId, tempIntersection)) {
            std::cerr << "Train " << trainId << " failed to send ACQUIRE request." << std::endl;
            return;
        }
        
        // Wait for response from the server
        std::string respIntersection;
        int response;

        // WAIT/DENY Handling
        while ((response = trainWaitForResponse(responseQueue, logQueue, trainId)) != ResponseType::GRANT) {
            if(response == ResponseType::WAIT) {
                // If WAIT, log and continue waiting
                sendLogMessage(logQueue, std::string(trainId) + ": Waiting for " + intersection + "...");
                sleep(1);
                simulatedTime++; // Update simulated time
            }
            else if (response == ResponseType::DENY) {
                // If DENY, log and exit
                sendLogMessage(logQueue, std::string(trainId) + ": DENIED access to " + intersection + ".");
                return;
            }

            else if (response == -1) {
                std::cerr << "Train " << trainId << " failed to receive response." << std::endl;
                return;
            }

        }
        
        // Intersection granted, simulate train crossing
        sendLogMessage(logQueue, std::string(trainId) + ": Acquired " + intersection + ". Proceeding...");
        
        // Simulate time to cross the intersection (2-5 seconds)
        int crossingTime = 2 + (rand() % 4);
        sleep(crossingTime);
        simulatedTime += crossingTime; // Update simulated time
        
        // Release the intersection
        if (!trainSendReleaseRequestExtended(requestQueue, logQueue, trainId, tempIntersection, shm, inter_ptr, sem, mutex, held)) {
            std::cerr << "Train " << trainId << " failed to send RELEASE request." << std::endl;
            return;
        }
    }
    
    sendLogMessage(logQueue, std::string(trainId) + ": Completed route.");
    trainSendDoneMsg(requestQueue, trainId);
    return;
}

/*
* Server functions for handling requests
*/

// Function to receive a request
bool serverReceiveRequest(int requestQueue, char* trainId, char* intersectionId, int& requestType) {
    RequestMsg req;
    
    // Receive any request message (both ACQUIRE and RELEASE)
    if (msgrcv(requestQueue, &req, sizeof(req) - sizeof(long), 0, 0) == -1) {
        if (errno == EINTR) {
            // Interrupted by signal
            return false;
        }
        std::cerr << "Failed to receive request: " << strerror(errno) << std::endl;
        return false;
    }
    
    strncpy(trainId, req.train_id, sizeof(req.train_id) - 1);
    trainId[sizeof(req.train_id) - 1] = '\0';
    strncpy(intersectionId, req.intersection_id, sizeof(req.intersection_id) - 1);
    intersectionId[sizeof(req.intersection_id) - 1] = '\0';
    requestType = req.mtype;
    
    simulatedTime++; // Update simulated time
    return true;
}


// Function to send a response
bool serverSendResponse(int responseQueue, int logQueue, const char* trainId, 
                        const char* intersectionId, int responseType) 
{
    ResponseMsg resp;

    std::string tempID = std::string(trainId);

    if (tempID.size() <= 5 || !std::all_of(tempID.begin() + 5, tempID.end(), ::isdigit)) {
        std::cerr << "Invalid train ID format: " << tempID << std::endl;
        return -1;
    }

    long trainIdLong = std::stol(tempID.substr(5));

    resp.mtype = trainIdLong;
    resp.response_type = responseType;
    strncpy(resp.intersection_id, intersectionId, sizeof(resp.intersection_id) - 1);
    resp.intersection_id[sizeof(resp.intersection_id) - 1] = '\0';
    
    if (msgsnd(responseQueue, &resp, sizeof(resp) - sizeof(long), 0) == -1) {
        std::cerr << "Failed to send response: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Log the response sent
    std::string responseTypeStr;
    switch (responseType) {
        case ResponseType::GRANT:
            responseTypeStr = "GRANTED";
            break;
        case ResponseType::WAIT:
            responseTypeStr = "WAIT";
            break;
        case ResponseType::DENY:
            responseTypeStr = "DENIED";
            break;
        default:
            responseTypeStr = "UNKNOWN";
    }
    
    if (responseType == ResponseType::GRANT) {

        sendLogMessage(logQueue, std::string("SERVER: ") + responseTypeStr + " " + intersectionId + " to " + trainId + ".");
    } else if (responseType == ResponseType::WAIT) {
        // log the wait. 
        sendLogMessage(logQueue, std::string("SERVER: ") + intersectionId + " is busy. " + trainId + " added to wait queue.");
    }
    
    simulatedTime++;
    return true;
}

// function to handle train requests (acquire or release or deny access to intersection)
void processTrainRequests(int requestQueue, int responseQueue, int logQueue, int waitQueue, shared_mem_t *shm, 
    Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex) {
    char trainId[16];
    char intersectionId[32];
    int reqType;
    int trainsDone = 0;
    char log[100]; // char array to hold log messages
    bool waitQueueProcessed = false;

    // Loop until msgrcv fails (e.g. when queue removed or signaled)
    while (trainsDone < shm->num_trains) {
        waitQueueProcessed = false;

        if(processWaitQueue(waitQueue, trainId, intersectionId)) {
            // Process wait queue
            // waiting trains are always trying to acquire the intersection

            // Grant the request
            if(!checkIntersectionFull(shm, inter_ptr, intersectionId, held)) {
                serverSendResponse(responseQueue, logQueue, trainId, intersectionId, ResponseType::GRANT);
                lockIntersection(shm, inter_ptr, sem, mutex, intersectionId, trainId, held);
                
            }
            else{
                addToWaitQueue(waitQueue, trainId, intersectionId);
                serverSendResponse(responseQueue, logQueue, trainId, intersectionId, ResponseType::WAIT);
                break;
            }
            waitQueueProcessed = true;
        }

        if(!waitQueueProcessed){
            if(!serverReceiveRequest(requestQueue, trainId, intersectionId, reqType)) {
                std::cerr << "processTrainRequests [ERROR]: Failed to receive request." << std::endl;
                continue;
            }

        if(reqType == RequestType::ACQUIRE) {
            // Grant the request
            if(checkIntersectionFull(shm, inter_ptr, intersectionId, held)) {
                serverSendResponse(responseQueue, logQueue, trainId, intersectionId, ResponseType::GRANT);
                lockIntersection(shm, inter_ptr, sem, mutex, intersectionId, trainId, held);

            }
            else{
                addToWaitQueue(waitQueue, trainId, intersectionId);
                serverSendResponse(responseQueue, logQueue, trainId, intersectionId, ResponseType::WAIT);
            }
        }
        else if (reqType == RequestType::RELEASE) {
            // release the interesction and log it.
            releaseIntersection(shm, inter_ptr, sem, mutex, intersectionId, trainId, held);
            sendLogMessage(logQueue, std::string("SERVER: ") + trainId + " released " + intersectionId + ".");
            
        }
        else if(reqType == RequestType::DONE) {
            // Log the completion
            trainsDone++;
            sendLogMessage(logQueue, std::string("SERVER: ") + trainId + " completed its route.");
        }
        else {
            std::cerr << "Unknown request type: " << reqType << std::endl;
        }

        
        }
        // take log message from queue and send to log file
        if(serverReceiveLog(logQueue, log)){
            logMessage(log);
        }
        

    }
}

//---------------------------------------- TrainCommunication.cpp ------------------------------------
/*
  Group G
  Author: Clayton Nunley
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
#include <sys/wait.h>   // Added for waitpid()

#include "TrainCommunication.h" // Added for message structure 


#include "sync.h" // included for semaphore and mutex implementation

// External file for logging
extern std::ofstream logFile;

// We define these in main.cpp (so only one definition in the whole project):
extern int simulatedTime; 

// Message structures for IPC
struct RequestMsg {
    long mtype;                 // Message type (1 for ACQUIRE, 2 for RELEASE)
    char train_id[20];          // Name of the train
    char intersection_id[20];   // Name of the intersection
};

struct ResponseMsg {
    long mtype;                 // Train ID as the message type for routing
    int response_type;          // 1 for GRANT, 2 for WAIT, 3 for DENY
    char intersection_id[20];   // Name of the intersection
};

// Constants for response types
namespace ResponseType {
    const int GRANT = 1;
    const int WAIT = 2;
    const int DENY = 3;
}

// Constants for request types
namespace RequestType {
    const int ACQUIRE = 1;
    const int RELEASE = 2;
}

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
    std::string timestamped = "[" + getTimestamp() + "] " + message;
    std::cout << timestamped << std::endl;
    if (logFile.is_open()) {
        logFile << timestamped << std::endl;
        logFile.flush();
    }
}

// Function to set up message queues
int setupMessageQueues(int& requestQueue, int& responseQueue) {
    key_t requestKey = ftok(".", 'R');
    key_t responseKey = ftok(".", 'S');
    
    requestQueue = msgget(requestKey, IPC_CREAT | 0666);
    responseQueue = msgget(responseKey, IPC_CREAT | 0666);
    
    if (requestQueue == -1 || responseQueue == -1) {
        std::cerr << "Failed to create message queues: " << strerror(errno) << std::endl;
        return -1;
    }
    
    return 0;
}

// Function to clean up message queues
void cleanupMessageQueues(int requestQueue, int responseQueue) {
    msgctl(requestQueue, IPC_RMID, nullptr);
    msgctl(responseQueue, IPC_RMID, nullptr);
}

//------------------------------------------------------------------------------
// Train functions for communicating with the server
//------------------------------------------------------------------------------

// Function to send an ACQUIRE request
bool trainSendAcquireRequest(int requestQueue, const std::string& trainId, const std::string& intersectionId) {
    RequestMsg msg;
    msg.mtype = RequestType::ACQUIRE;
    strncpy(msg.train_id, trainId.c_str(), sizeof(msg.train_id) - 1);
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';
    strncpy(msg.intersection_id, intersectionId.c_str(), sizeof(msg.intersection_id) - 1);
    msg.intersection_id[sizeof(msg.intersection_id) - 1] = '\0';
    
    if (msgsnd(requestQueue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        std::cerr << "Failed to send ACQUIRE request: " << strerror(errno) << std::endl;
        return false;
    }
    
    logMessage("TRAIN" + trainId + ": Sent ACQUIRE request for " + intersectionId + ".");
    return true;
}

// Function to send a RELEASE request
bool trainSendReleaseRequest(int requestQueue, const std::string& trainId, const std::string& intersectionId) {
    RequestMsg msg;
    msg.mtype = RequestType::RELEASE;
    strncpy(msg.train_id, trainId.c_str(), sizeof(msg.train_id) - 1);
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';
    strncpy(msg.intersection_id, intersectionId.c_str(), sizeof(msg.intersection_id) - 1);
    msg.intersection_id[sizeof(msg.intersection_id) - 1] = '\0';
    
    if (msgsnd(requestQueue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        std::cerr << "Failed to send RELEASE request: " << strerror(errno) << std::endl;
        return false;
    }
    else {
        // Log the release request
        releaseIntersection(intersectionId);
        logMessage("TRAIN" + trainId + ": Sent RELEASE request for " + intersectionId + ".");
        return true;
    }
    
    // in case of unexpected errors
    return false;
}

// Function for trains to wait for a response from the server
int trainWaitForResponse(int responseQueue, const std::string& trainId, std::string& intersectionId) {
    ResponseMsg msg;
    
    // Convert train_id to a long for message filtering
    long trainIdLong = std::stol(trainId);
    
    // Receive response message specifically for this train
    if (msgrcv(responseQueue, &msg, sizeof(msg) - sizeof(long), trainIdLong, 0) == -1) {
        std::cerr << "Failed to receive response: " << strerror(errno) << std::endl;
        return -1;
    }
    
    intersectionId = std::string(msg.intersection_id);
    
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
    
    logMessage("TRAIN" + trainId + ": Received " + responseTypeStr + " for " + intersectionId + ".");
    
    return msg.response_type;
}

// Function to simulate train movement
void simulateTrainMovement(const std::string& trainId, const std::vector<std::string>& route, 
                           int requestQueue, int responseQueue) 
{
    // Iterate through each intersection in the route
    for (const auto& intersection : route) {
        // Request to acquire the intersection
        if (!trainSendAcquireRequest(requestQueue, trainId, intersection)) {
            std::cerr << "Train " << trainId << " failed to send ACQUIRE request." << std::endl;
            return;
        }
        
        // Wait for response from the server
        std::string respIntersection;
        int response;
        /*
        // Simplified: always wait for a GRANT (in a more complex system, you'd handle WAIT/DENY properly)
        while ((response = trainWaitForResponse(responseQueue, trainId, respIntersection)) != ResponseType::GRANT) {
            if (response == -1) {
                std::cerr << "Train " << trainId << " failed to receive response." << std::endl;
                return;
            }
            // If not granted, wait a bit and continue waiting
            logMessage("TRAIN" + trainId + ": Waiting for " + intersection + "...");
            sleep(1);
            simulatedTime++; // Update simulated time
        }
            */
        // WAIT/DENY Handling
        while ((response = trainWaitForResponse(responseQueue, trainId, respIntersection)) != ResponseType::GRANT) {
            if(resposne == ResponseType::WAIT) {
                // If WAIT, log and continue waiting
                logMessage("TRAIN" + trainId + ": Waiting for " + intersection + "...");
                sleep(1);
                simulatedTime++; // Update simulated time
            }
            else if (response == ResponseType::DENY) {
                // If DENY, log and exit
                logMessage("TRAIN" + trainId + ": DENIED access to " + intersection + ".");
                return;
            }

            if (response == -1) {
                std::cerr << "Train " << trainId << " failed to receive response." << std::endl;
                return;
            }

        }
        
        // Intersection granted, simulate train crossing
        logMessage("TRAIN" + trainId + ": Acquired " + intersection + ". Proceeding...");
        
        // Simulate time to cross the intersection (2-5 seconds)
        int crossingTime = 2 + (rand() % 4);
        sleep(crossingTime);
        simulatedTime += crossingTime; // Update simulated time
        
        // Release the intersection
        if (!trainSendReleaseRequest(requestQueue, trainId, intersection)) {
            std::cerr << "Train " << trainId << " failed to send RELEASE request." << std::endl;
            return;
        }
    }
    
    logMessage("TRAIN" + trainId + ": Completed route.");
}

//------------------------------------------------------------------------------
// Server functions for handling requests
//------------------------------------------------------------------------------

// Function to receive a request
bool serverReceiveRequest(int requestQueue, std::string& trainId, std::string& intersectionId, int& requestType) {
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
    
    trainId = std::string(req.train_id);
    intersectionId = std::string(req.intersection_id);
    requestType = req.mtype;
    
    simulatedTime++; // Update simulated time
    return true;
}

// Function to send a response
bool serverSendResponse(int responseQueue, const std::string& trainId, 
                        const std::string& intersectionId, int responseType) 
{
    ResponseMsg resp;
    resp.mtype = std::stol(trainId); // Route response to specific train
    resp.response_type = responseType;
    strncpy(resp.intersection_id, intersectionId.c_str(), sizeof(resp.intersection_id) - 1);
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

        logMessage("SERVER: " + responseTypeStr + " " + intersectionId + " to " + trainId + ".");
    } else if (responseType == ResponseType::WAIT) {
        logMessage("SERVER: " + intersectionId + " is busy. " + trainId + " added to wait queue.");
    }
    
    simulatedTime++; // Update simulated time
    return true;
}

// Simplified server side: always grants ACQUIRE, logs RELEASE
void processTrainRequests(int requestQueue, int responseQueue) {
    std::string trainId, intersectionId;
    int reqType;
    
    // Loop until msgrcv fails (e.g. when queue removed or signaled)
    while (serverReceiveRequest(requestQueue, trainId, intersectionId, reqType)) {
        if (reqType == RequestType::ACQUIRE) {
            if(checkIntersectionLock(intersectionId)){
                lockIntersection(intersectionId);
                serverSendResponse(responseQueue, trainId, intersectionId, ResponseType::GRANT);
            }
            else if(!checkIntersectionLock(intersectionId)){
                serverSendResponse(responseQueue, trainId, intersectionId, ResponseType::WAIT);
            }
            else{
                serverSendResponse(responseQueue, trainId, intersectionId, ResponseType::DENY);
            }           
        }
        else if (reqType == RequestType::RELEASE) {
            // Just log it
            logMessage("SERVER: " + trainId + " released " + intersectionId + ".");
        }
    }
}

// Spawns child processes for each train route
std::vector<pid_t> forkTrainProcesses(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& trainRoutes,
    int requestQueue, int responseQueue)
{
    std::vector<pid_t> trainPids;
    
    for (const auto& trainData : trainRoutes) {
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
            return trainPids;
        }
        if (pid == 0) {
            // Child process (this train)
            simulateTrainMovement(trainData.first, trainData.second, requestQueue, responseQueue);
            exit(EXIT_SUCCESS);
        } else {
            trainPids.push_back(pid);
        }
    }
    
    return trainPids;
}

//------------------------------------------------------------------------------
// A small "Week 1 test" function that sets everything up and demonstrates it
//------------------------------------------------------------------------------

void runClaytonWeek1Test() 
{
    std::cout << "\n--- Demonstrating Clayton’s Train Communication logic ---\n";
    
    // Open a dedicated log file for demonstration
    logFile.open("Clayton_Week1.log");
    if (!logFile.is_open()) {
        std::cerr << "Could not open Clayton_Week1.log for logging.\n";
    }

    // Set up message queues
    int requestQ = 0, responseQ = 0;
    if (setupMessageQueues(requestQ, responseQ) == -1) {
        std::cerr << "Could not set up message queues.\n";
        return;
    }

    // Fork the server in a child process
    pid_t serverPid = fork();
    if (serverPid < 0) {
        std::cerr << "Fork for server process failed.\n";
        return;
    }
    if (serverPid == 0) {
        // Child: act as the server
        processTrainRequests(requestQ, responseQ);
        exit(0);
    }
    else {
        // Parent: fork multiple trains
        srand(time(NULL));
        std::vector<std::pair<std::string, std::vector<std::string>>> trainRoutes {
            // For a quick demo, create two trains with short routes
            {"1", {"IntersectionA", "IntersectionB"}},
            {"2", {"IntersectionB", "IntersectionC"}}
        };

        auto trainPids = forkTrainProcesses(trainRoutes, requestQ, responseQ);

        // Wait for all trains to finish
        for (pid_t pid : trainPids) {
            waitpid(pid, nullptr, 0);
        }
        
        // At this point, no more train requests, so we can remove queues.
        // Removing the queue will cause the server’s msgrcv to fail and exit.
        cleanupMessageQueues(requestQ, responseQ);

        // Wait for the server to notice the queue is gone and exit
        waitpid(serverPid, nullptr, 0);

        // Close log file
        if (logFile.is_open()) {
            logFile.close();
        }
        std::cout << "--- End of Clayton’s Train Communication demo ---\n";
    }
}

/*
Group: G
Author: Damian Silvar
Email: damian.silvar@okstate.edu
Date: 04-13-2025
*/

#ifndef TRAIN_COMMUNICATION_H
#define TRAIN_COMMUNICATION_H

#include <string>
#include <vector>
#include <fstream>

// Message structures for IPC
struct RequestMsg {
    long mtype;                  // Message type
    char train_id[20];            // Train ID
    char intersection_id[20];     // Intersection ID
};

struct ResponseMsg {
    long mtype;                  // Message type
    int response_type;           // Grant, Wait, or Deny
    char intersection_id[20];    // Intersection ID
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

// Function declarations

std::string getTimestamp();
void logMessage(const std::string& message);

// Setup and Cleanup
int setupMessageQueues(int& requestQueue, int& responseQueue);
void cleanupMessageQueues(int requestQueue, int responseQueue);

// Train side
bool trainSendAcquireRequest(int requestQueue, const std::string& trainId, const std::string& intersectionId);
bool trainSendReleaseRequest(int requestQueue, const std::string& trainId, const std::string& intersectionId, 
    shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, int *held);
    
int trainWaitForResponse(int responseQueue, const std::string& trainId, std::string& intersectionId);
void simulateTrainMovement(const std::string& trainId, const std::vector<std::string>& route, int requestQueue, int responseQueue, shared_mem_t *shm,
     Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex);

// Server side
bool serverReceiveRequest(int requestQueue, std::string& trainId, std::string& intersectionId, int& requestType);
bool serverSendResponse(int responseQueue, const std::string& trainId, const std::string& intersectionId, int responseType);
void processTrainRequests(int requestQueue, int responseQueue, shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex);

// External logging file and simulated time
extern std::ofstream logFile;
extern int simulatedTime;

#endif

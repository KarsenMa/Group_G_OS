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

#include <semaphore.h>
#include <pthread.h>
#include "sync.h" // included for semaphore and mutex implementation
#include "shared_Mem.h" // included for shared memory implementation

// Message structures
struct RequestMsg {
    long mtype;                  // Message type
    char train_id[20];            // Train ID
    char intersection_id[20];     // Intersection ID
};

struct ResponseMsg {
    long mtype;                  // Message type
    int response_type;           // will be either grant, wait, or deny
    char intersection_id[20];    
};

// Constants per response types
namespace ResponseType {
    const int GRANT = 1;
    const int WAIT = 2;
    const int DENY = 3;
}

// Constants per request types
namespace RequestType {
    const int ACQUIRE = 1;
    const int RELEASE = 2;
    const int DONE = 3;
}

// Functions

std::string getTimestamp();
void logMessage(const std::string& message);

// Setup and Cleanup
int setupMessageQueues(int& requestQueue, int& responseQueue);
void cleanupMessageQueues(int requestQueue, int responseQueue);

// Train side
bool trainSendAcquireRequest(int requestQueue, const char* trainId, const char* intersectionId);
bool trainSendReleaseRequest(int requestQueue, const char* trainId, const char* intersectionId, 
    shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex, int *held);
bool trainSendDoneMsg(int requestQueue, const char* trainId);

int trainWaitForResponse(int responseQueue, const char* trainId, const char* intersectionId);
void simulateTrainMovement(const char* trainId, const std::vector<std::string>& route, int requestQueue, int responseQueue, shared_mem_t *shm,
     Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex);

// Server side
bool serverReceiveRequest(int requestQueue, const char* trainId, const char* intersectionId, int& requestType);
bool serverSendResponse(int responseQueue, const char* trainId, const char* intersectionId, int responseType);
void processTrainRequests(int requestQueue, int responseQueue, shared_mem_t *shm, Intersection *inter_ptr, int *held, sem_t *sem, pthread_mutex_t *mutex);

// Logging file and simulated time
extern std::ofstream logFile;
extern int simulatedTime;

#endif

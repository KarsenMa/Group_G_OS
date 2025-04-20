/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/20/2025
    Program Description: This file contains methods to implement wait queueing, 
    a log queue to synchronize the simulation log, and a function to send a DONE
    message to the server.
*/
#ifndef TRAIN_COMM_EXTENSION_H
#define TRAIN_COMM_EXTENSION_H

// Message structure for logging and wait queueing
struct LogMsg { 
    long mtype; 
    char message[100];
};

struct WaitQueueMsg {
    // long mtype; 
    char train_id[20]; 
    char intersection_id[20];
};



bool trainSendDoneMsg(int requestQueue, const char* trainId);

bool serverReceiveLog(int logQueue, char* log);

void addToWaitQueue(int waitQueue, const char* trainId, const char* intersectionId);

bool processWaitQueue(int waitQueue, const char* trainId, const char* intersectionId, int logQueue, shared_mem_t *shm, Intersection *inter_ptr, sem_t *sem, pthread_mutex_t *mutex);

#endif
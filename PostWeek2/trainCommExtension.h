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

#include <cstring>
#include <iostream>
#include <sys/msg.h>

#include "shared_Mem.h"
#include "TrainCommunication.h"

// Message structure for logging and wait queueing
struct LogMsg { 
    long mtype; 
    char message[100];
};

struct WaitQueueMsg {
    long mtype; 
    char train_id[20]; 
    char intersection_id[20];
};



bool trainSendDoneMsg(int requestQueue, const char* trainId);

bool serverReceiveLog(int logQueue, char* log);

bool addToWaitQueue(int waitQueue, const char* trainId, const char* intersectionId);

bool processWaitQueue(int waitQueue,  char* trainId, char* intersectionId);

#endif
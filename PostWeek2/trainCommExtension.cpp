/*  Group G
    Author Name: Cosette Byte
    Email: cosette.byte@okstate.edu
    Date: 4/20/2025
    Program Description: This file contains methods to implement wait queueing, 
    a log queue to synchronize the simulation log, and a function to send a DONE
    message to the server.
*/

#include <cstring>
#include <iostream>
#include <sys/msg.h>

#include "shared_Mem.h"
#include "trainCommExtension.h"
#include "TrainCommunication.h"



/* Function to send a DONE message keeping process running in main until all train are done
*  this function takes a requestQueue and a char buffer as input. The requestQueue is a queue that holds the
*  request messages, and the char buffer is where the request message will be copied to.
*  this function returns a bool that indicates if the DONE message was sent. 
*/
bool trainSendDoneMsg(int requestQueue, const char* trainId){
    RequestMsg msg;
        
        msg.mtype = RequestType::DONE;
        // copy train ID to request message
        strncpy(msg.train_id, trainId, sizeof(msg.train_id) - 1);
        msg.train_id[sizeof(msg.train_id) - 1] = '\0';
        
        // Send DONE message to the server
        if (msgsnd(requestQueue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
            std::cerr << "Failed to send DONE message: " << strerror(errno) << std::endl;
            return false;
        }
        return true;
}

/* function to have the server receive a log message and copy it to the log char buffer
*  this function takes a logQueue and a char buffer as input. The logQueue is a queue that holds the 
*  log messages, and the char buffer is where the log message will be copied to.
*  the function returns a bool that indicates if the server received the log message.
*/
bool serverReceiveLog(int logQueue, char* log) { 
    LogMsg logMsg;

    // Receive log message
    if (msgrcv(logQueue, &logMsg, sizeof(logMsg) - sizeof(long), 0, 0) == -1) {
        if(errno == EINTR) {
            // Interrupted by signal
            return false;
        }
        std::cerr << "serverReceiveLog [ERROR]: Failed to receive log message: " << strerror(errno) << std::endl;
        return false;
    }
    // copy the log message to the char buffer
    strncpy(log, logMsg.message, sizeof(logMsg.message) - 1);
    log[sizeof(logMsg.message) - 1] = '\0';

    return true;
}


/* function to add a train to the wait queue and give the intersection ID of the intersection 
*  that the train is waiting for. 
*  this function takes the waitQueue, the train ID and the intersection ID as input.
*  it returns a bool that indicates if the message was sent. 
*/
bool addToWaitQueue(int waitQueue, const char* trainId, const char* intersectionId) {
    WaitQueueMsg waitMsg;

    std::string tempID = std::string(trainId);
    long trainIdLong = std::stol(tempID.substr(5));

    waitMsg.mtype = trainIdLong; 
    // copy train ID and intersection ID to wait message
    strncpy(waitMsg.train_id, trainId, sizeof(waitMsg.train_id) - 1);
    waitMsg.train_id[sizeof(waitMsg.train_id) - 1] = '\0';

    strncpy(waitMsg.intersection_id, intersectionId, sizeof(waitMsg.intersection_id) - 1);  
    waitMsg.intersection_id[sizeof(waitMsg.intersection_id) - 1] = '\0';

    // send the wait message to the wait queue
    if(msgsnd(waitQueue, &waitMsg, sizeof(waitMsg) - sizeof(long), 0) == -1) {
        std::cerr << "addToWaitQueue [ERROR]: Failed to add to wait queue" << std::endl;
        return false;
    } 

    return true;
}

/* function to receive wait message from the wait queue. Copies the train ID and intersection ID to the char buffer
* this functions takes the waitQueue, the train ID and the intersection ID as input.
* it returns a bool that indicates if the wait message was received.
*/
bool processWaitQueue(int waitQueue, char* trainId, char* intersectionId) {
    WaitQueueMsg waitMsg;

    // Receive wait message
    if (msgrcv(waitQueue, &waitMsg, sizeof(waitMsg) - sizeof(long), 0, IPC_NOWAIT) == -1) {
        std::cerr << "processWaitQueue [ERROR]: Failed to receive wait message: " << strerror(errno) << std::endl;
        return false;
    }

    // Copy train ID and intersection ID to the char buffer
    strncpy(trainId, waitMsg.train_id, sizeof(waitMsg.train_id) - 1);
    trainId[sizeof(waitMsg.train_id) - 1] = '\0';

    strncpy(intersectionId, waitMsg.intersection_id, sizeof(waitMsg.intersection_id) - 1);
    intersectionId[sizeof(waitMsg.intersection_id) - 1] = '\0';

    return true; // wait message was received
}


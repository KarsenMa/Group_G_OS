/*
Group: G
Author: Damian Silvar
Email: damian.silvar@okstate.edu
Date: 04-11-2025
*/

#ifndef IPCSEMAPHORE_H
#define IPCSEMAPHORE_H

#include <semaphore.h>   // For POSIX semaphore functions
#include <string>        // For std::string

// Class to manage a named POSIX semaphore for interprocess communication
class IPCSemaphore {
private:
    sem_t* sem;          // Pointer to semaphore
    std::string name;    // Name of semaphore

public:
    // Creates or opens a named semaphore with an initial value
    IPCSemaphore(const std::string& semName, unsigned int initialValue);

    // Closes and unlinks the semaphore
    ~IPCSemaphore();

    // Locks the semaphore
    void wait();

    // Unlocks the semaphore
    void post();
};

#endif


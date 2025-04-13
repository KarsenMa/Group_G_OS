/*
Group: G
Author: Damian Silvar
Email: damian.silvar@okstate.edu
Date: 04-11-2025
*/

#ifndef MUTEXTHREAD_H
#define MUTEXTHREAD_H

#include <semaphore.h>   // For POSIX semaphore functions
#include <string>        // For std::string

// Class to manage a named semaphore as a mutex
class MutexThread {
private:
    sem_t* mutex;        // Pointer to semaphore (mutex)
    std::string name;    // Name of semaphore

public:
    // Creates or opens a named semaphore
    MutexThread(const std::string& mutexName);

    // Closes and unlinks the semaphore
    ~MutexThread();

    // Locks, waits, the semaphore
    void lock();

    // Unlocks, posts, the semaphore
    void unlock();
};

#endif

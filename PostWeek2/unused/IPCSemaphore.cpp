/*
Group: G
Author: Damian Silvar
Email: damian.silvar@okstate.edu
Date: 04-11-2025
Description: Manages an IPC semaphore using POSIX semaphores. Provides wait and post operations to synchronize processes.
*/

#include "IPCSemaphore.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>

IPCSemaphore::IPCSemaphore(const std::string& semName, unsigned int initialValue) : name(semName) {
    // Creates or open the semaphore with specified name and initial value
    sem = sem_open(name.c_str(), O_CREAT, 0644, initialValue);
    if (sem == SEM_FAILED) {
        perror("sem_open failed for IPCSemaphore");
    }
}

IPCSemaphore::~IPCSemaphore() {
    // Closes and unlinks the semaphore when no longer needed
    if (sem != SEM_FAILED) {
        sem_close(sem);
        sem_unlink(name.c_str()); // Cleanup the semaphore
    }
}

void IPCSemaphore::wait() {
    // Wait operation to acquire the semaphore and decrement value
    if (sem_wait(sem) == -1) {
        perror("sem_wait failed");
    }
}

void IPCSemaphore::post() {
    // Post operation to release the semaphore and increment the value
    if (sem_post(sem) == -1) {
        perror("sem_post failed");
    }
}

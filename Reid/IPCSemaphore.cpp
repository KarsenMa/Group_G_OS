
//Author: Damian Silvar
//Email: damian.silvar
//Date: 4-2-25

#include "IPCSemaphore.h"
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t sem; // Semaphore object

// A simple worker function to demonstrate semaphore usage
void* worker(void* arg) { 
    int id = *((int*)arg);
    sem_wait(&sem);
    std::cout << "Damian's Process " << id << " accessing shared resource\n";
    sleep(1);
    sem_post(&sem);
    return nullptr;
}

void startProcesses() {
    std::cout << "\n--- Demonstrating Damian's semaphore code ---\n";
    sem_init(&sem, 0, 1);

    pthread_t t1, t2;
    int id1 = 1, id2 = 2;

    if (pthread_create(&t1, nullptr, worker, &id1) != 0) {
        std::cerr << "Failed to create thread 1\n";
    }
    if (pthread_create(&t2, nullptr, worker, &id2) != 0) {
        std::cerr << "Failed to create thread 2\n";
    }

    pthread_join(t1, nullptr);
    pthread_join(t2, nullptr);

    sem_destroy(&sem);
}

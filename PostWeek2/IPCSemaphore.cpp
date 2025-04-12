//Author: Damian Silvar
//Email: damian.silvar@okstate.edu
//Date: 4-11-25

#include <algorithm>  
#include "IPCSemaphore.h"
#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>

sem_t sem;
std::vector<int> process_waiting_for;  // Tracks which process is waiting for which resource (semaphore)
std::vector<int> process_holding;      // Tracks which process is holding which resource

// --- UPDATED: Deadlock Detection ---

void detectProcessDeadlock() {
    // Simple cycle detection for deadlock using the wait-for graph
    for (size_t i = 0; i < process_waiting_for.size(); ++i) {        // goes through process i to see if waiting for resource by process j
        for (size_t j = 0; j < process_waiting_for.size(); ++j) {    //    and vice versa
            if (process_waiting_for[i] == process_holding[j] && process_waiting_for[j] == process_holding[i]) {  // will detect deadlock
                std::cout << "Deadlock detected between Process " << i << " and Process " << j << std::endl;
                return;
            }
        }
    }
}

void* worker(void* arg) {
    int id = *((int*)arg);
    sem_wait(&sem);

    process_holding.push_back(id);  // Process holds the semaphore
    std::cout << "Process " << id << " accessing shared resource\n";
    sleep(1);
    process_holding.erase(std::remove(process_holding.begin(), process_holding.end(), id), process_holding.end());  // Releases semaphore
    sem_post(&sem);

    std::cout << "Process " << id << " released shared resource\n";

    return nullptr;
}

void startProcesses() {
    sem_init(&sem, 0, 1);

    pthread_t t1, t2;      // Thread identifiers
    int id1 = 1, id2 = 2;  // ID thread

    if (pthread_create(&t1, nullptr, worker, &id1) != 0) {   // first thread created
        std::cerr << "Failed to create thread 1\n";
    }

    if (pthread_create(&t2, nullptr, worker, &id2) != 0) {   // second thread created
        std::cerr << "Failed to create thread 2\n";
    }

    pthread_join(t1, nullptr);    // waits for threads to join
    pthread_join(t2, nullptr);

    detectProcessDeadlock();     // checks deadlocks after threads are completed

    sem_destroy(&sem);  // Cleans up semaphore
}

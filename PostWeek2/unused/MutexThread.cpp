/*
Group: G
Author: Damian Silvar
Email: damian.silvar@okstate.edu
Date: 04-11-2025
Description: Simulates two threads accessing a shared critical section with mutexes.
After each thread completes its work, the system checks for deadlocks using the detectAndResolveDeadlock function.
*/

#include <algorithm>
#include "MutexThread.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include "DeadlockDetection.h"

std::mutex mtx;
std::vector<int> thread_waiting_for;  // Tracks which thread is waiting for which resource (mutex)
std::vector<int> thread_holding;      // Tracks which thread is holding which resource

// critical section
void criticalSection(int id, shared_mem_t* shm, const std::vector<Intersection>& intersections) {
    {
        std::lock_guard<std::mutex> lock(mtx);

        // Tracks which thread is holding which resource (mutex)
        thread_holding.push_back(id);
        std::cout << "Thread " << id << " is in the critical section\n";

        // Critical section work
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // After work, release the resource (mutex)
        thread_holding.erase(std::remove(thread_holding.begin(), thread_holding.end(), id), thread_holding.end());
    }

    // After critical section, checks deadlocks using the deadlock detection
    std::string cycleDescription;
    detectAndResolveDeadlock(shm, intersections);  // Calls deadlock detection from DeadlockDetection.cpp
    std::cout << "Thread " << id << " is leaving the critical section\n";
}

// Starts threads and detects deadlocks
void startThreads(shared_mem_t* shm, const std::vector<Intersection>& intersections) {
    std::thread t1(criticalSection, 1, shm, std::ref(intersections));
    std::thread t2(criticalSection, 2, shm, std::ref(intersections));

    t1.join();
    t2.join();

    // Calls deadlock detection after threads complete
    detectAndResolveDeadlock(shm, intersections);
}

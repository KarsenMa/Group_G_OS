// Group G
//Author: Damian Silvar
//Email: damian.silvar@okstate.edu
//Date: 4-11-25

#include <algorithm>  
#include "MutexThread.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> thread_waiting_for;  // Tracks which thread is waiting for which resource (mutex)
std::vector<int> thread_holding;      // Tracks which thread is holding which resource

// --- UPDATED: Deadlock Detection ---

void detectThreadDeadlock() {
    // Simple cycle detection for deadlock using the wait-for graph.
    for (size_t i = 0; i < thread_waiting_for.size(); ++i) {       // goes through process i to see if waiting for resource by process j
        for (size_t j = 0; j < thread_waiting_for.size(); ++j) {   //    and vice versa
            if (thread_waiting_for[i] == thread_holding[j] && thread_waiting_for[j] == thread_holding[i]) {  // will detect deadlock
                std::cout << "Deadlock detected between Thread " << i << " and Thread " << j << std::endl;
                return;
            }
        }
    }
}

void criticalSection(int id) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        thread_holding.push_back(id);  // Thread is holding the mutex
        std::cout << "Thread " << id << " is in the critical section\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        thread_holding.erase(std::remove(thread_holding.begin(), thread_holding.end(), id), thread_holding.end());  // Release mutex
    }

    std::cout << "Thread " << id << " is leaving the critical section\n";
}

void startThreads() {
    std::thread t1(criticalSection, 1);
    std::thread t2(criticalSection, 2);

    t1.join();
    t2.join();

    detectThreadDeadlock();
}

//Author: Damian Silvar
//Email: damian.silvar@okstate.edu
//Date: 4-2-25

#include "MutexThread.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mtx;

void criticalSection(int id) {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Damian's Thread " << id << " is in the critical section\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Damian's Thread " << id << " is leaving the critical section\n";
}

void startThreads() {
    std::cout << "\n--- Demonstrating Damian's mutex code ---\n";
    std::thread t1(criticalSection, 1);
    std::thread t2(criticalSection, 2);
    t1.join();
    t2.join();
}

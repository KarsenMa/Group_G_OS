#include <fstream> // Compiler is being very picky about this include
/*  
  Group G
  Author Name: Reid Wilson
  Email: reid.wilson@okstate.edu
  Date: 4/6/2025

  Program Description: Combined "Week 1" demonstration main
  You can compile this with:g++ main.cpp IPCSemaphore.cpp MutexThread.cpp trainFiles.cpp Forking_Trains.cpp TrainCommunication.cpp shared_Mem.cpp -lpthread -lrt -o week1_demo

  Notes: A lot of the other files had to be modified to work with this main file, as we all tested separartely.
  This is a combined demo for all the group members to show their work.
  - Reid Wilson
*/

// These are declared in TrainCommunication.cpp as "extern"
std::ofstream logFile;
int simulatedTime = 0; // Keep one global definition

#include <iostream>
#include <cstdlib>
#include "MutexThread.h"
#include "IPCSemaphore.h"

// Eric’s file parsing (declared in trainFiles.cpp)
void testEricFileParsing();

// Karsen’s forking demo
void demoKarsenForking();

// Clayton’s basic message queue demo
void runClaytonWeek1Test();

// Cosette’s shared memory demo
#include "shared_Mem.h"

int main() {
    std::cout << "=== Week 1 Combined Demo ===\n";

    // 1. Damian's simple demos
    startThreads();    // from MutexThread.cpp
    startProcesses();  // from IPCSemaphore.cpp

    // 2. Eric's file parsing test
    //    Make sure intersections.txt and trains.txt exist in same directory
    testEricFileParsing();

    // 3. Karsen's forking example
    demoKarsenForking();

    // 4. Clayton’s message-queue train logic
    runClaytonWeek1Test();

    // 5. Cosette’s shared memory test
    {
        std::cout << "\n--- Demonstrating Cosette's shared memory setup ---\n";
        shared_Mem mem;
        void *ptr = mem.mem_setup();
        if (!ptr) {
            std::cerr << "Failed to set up shared memory.\n";
        } else {
            std::cout << "Shared memory allocated successfully.\n";
            mem.mem_close(ptr);
            std::cout << "Shared memory closed/unlinked.\n";
        }
    }

    std::cout << "\n=== End of Week 1 Demo ===\n\n";
    return 0;
}

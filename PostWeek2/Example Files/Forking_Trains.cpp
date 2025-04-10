/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   03-31-2025
*/

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <cstdlib>

using namespace std;

struct Train {
    string name;
    vector<string> route;
};

// Example test data
static vector<Train> testTrains = {
    {"Train1", {"IntersectionA", "IntersectionB", "IntersectionC"}},
    {"Train2", {"IntersectionC", "IntersectionD", "IntersectionE"}}
};

/**
 * demoKarsenForking:
 *   A function we can call from main() to demonstrate Karsen’s child processes.
 */
void demoKarsenForking() {
    cout << "\n--- Demonstrating Karsen's forking code ---\n";

    vector<pid_t> pids;
    for (size_t i = 0; i < testTrains.size(); ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            cerr << testTrains[i].name << " Error: fork failed.\n";
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Child
            cout << testTrains[i].name << " Started\n";
            for (const auto &intersection : testTrains[i].route) {
                cout << testTrains[i].name << " At " << intersection << endl;
                sleep(1); // Simulate travel time
            }
            cout << testTrains[i].name << " Finished\n";
            exit(0);
        } else {
            // Parent
            pids.push_back(pid);
        }
    }

    // Wait for all
    for (auto &pid : pids) {
        waitpid(pid, nullptr, 0);
    }

    cout << "All Trains Finished\n";
}

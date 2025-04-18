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

struct Train
{
    string name;
    vector<string> route;
};

/* Test data prior to parsing data - waiting on group members*/
static vector<Train> testTrains = {
    {"Train1", {"IntersectionC", "IntersectionB", "IntersectionD"}},
    {"Train2", {"IntersectionB", "IntersectionD", "IntersectionE"}}};

/**
 * demoKarsenForking:
 *   A function we can call from main() to demonstrate Karsen’s child processes.
 */
void demoKarsenForking()
{
    cout << "\nDemonstrating Karsen's forking code\n";

    vector<pid_t> pids;
    for (size_t i = 0; i < testTrains.size(); ++i)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            cerr << testTrains[i].name << "fork failed\n";
            exit(EXIT_FAILURE);
        }
        if (pid == 0)
        {
            // Child
            cout << testTrains[i].name << " Started\n";
            for (const auto &intersection : testTrains[i].route)
            {
                cout << testTrains[i].name << " At " << intersection << endl;
                sleep(1); // Simulate travel time
            }
            cout << testTrains[i].name << " Finished\n";
            exit(0);
        }
        else
        {
            // Parent
            pids.push_back(pid);
        }
    }

    // Wait for all
    for (auto &pid : pids)
    {
        waitpid(pid, nullptr, 0);
    }

    cout << "All Trains Finished\n";
}

/*Task: Create function to fork process for trains that is callable frm main function*/
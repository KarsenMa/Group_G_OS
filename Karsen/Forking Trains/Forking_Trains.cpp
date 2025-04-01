/*
Group:  G
Author: Karsen Madole
Email:  kmadole@okstate.edu
Date:   03-31-2025
*/

// Libraries Needed for Program
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <string>

using namespace std;

struct Train // Holds the train name and route
{
    string name;
    vector<string> route;
};

// Train Test Data: Remove for actual data later
vector<Train> trains = {
    {"Train1", {"IntersectionA", "IntersectionB", "IntersectionC"}},
    {"Train2", {"IntersectionC", "IntersectionD", "IntersectionE"}},
    {"Train3", {"IntersectionD", "IntersectionE", "IntersectionF"}},
    {"Train4", {"IntersectionF", "IntersectionA", "IntersectionB"}}};

int main()
{
    vector<pid_t> pids; // Store Trains PIDs

    for (size_t i = 0; i < trains.size(); ++i) // Create All Trains
    {
        pid_t pid = fork(); // Create Train

        if (pid < 0)
        {
            // Alert User If Train Failed
            cerr << "Train: " << trains[i].name << "Error" << endl;
            exit(EXIT_FAILURE);
        }

        if (pid == 0) // Execute for Each Train
        {
            // Testing Perposes Show Train Started
            cout << "Train: " << trains[i].name << " Started" << endl;

            for (const string &intersection : trains[i].route) // Simulate Travelling Through Intersections
            {
                cout << "Train: " << trains[i].name << " At " << intersection << endl;
                sleep(1); // Simulate Travel Time
            }
            cout << "Train: " << trains[i].name << " Finished" << endl;
            exit(0); // Terminate Train
        }
        else
        {
            // Store the PID of the child process
            pids.push_back(pid);
        }
    }

    // Wait for all trains to finish
    for (pid_t pid : pids)
    {
        waitpid(pid, nullptr, 0);
    }

    cout << "All Trains Finished!" << endl;
    return 0;
}

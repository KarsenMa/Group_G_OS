/*
Group Number: G
Author: Eric Vo
Email: eric.t.vo@okstate.edu
Date: 13 April 2025
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "TrainFileParser.h"

std::ofstream logFile;      // Global log file stream for logging simulation output
int simulatedTime = 0;      // Variable to track simulation time

int main() {


    // Initialize the log file and opens the "simulation.log" file for logMessage in TrainCommunication.cpp to write in
    logFile.open("simulation.log", std::ios::out);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open simulation.log for writing." << std::endl;
        return -1;
    }

    // Load train and intersection data
    std::map<std::string, std::vector<std::string>> intersections;
    std::map<std::string, std::vector<std::string>> trains;
    parseFile("intersections.txt", intersections);  // Parse intersection data
    parseFile("trains.txt", trains);                // Parse train data

    // Displays the contents inside the maps for intersections and trains (optional)
    displayData(intersections, "Intersections and Capacities");
    displayData(trains, "Trains and Routes");

    // Logs when the system is first initalized
    logFile << "[00:00:00] SERVER: Initialized intersections:\n";

    // Server side messaging stuff

    // Close the log file before exiting
    logFile.close();

    return 0;
}

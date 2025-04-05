/*
Group Number: G
Author: Eric Vo
Email: eric.t.vo@okstate.edu
Date: 6 April 2025
*/

// Required libraries
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Reads and pareses the two text files (intersections.txt and trains.txt)
void parseFile(const string& filename, unordered_map<string, vector<string>>& data_map) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string item_name = line.substr(0, colon_pos);
            string all_values = line.substr(colon_pos + 1);

            // Split the value into a vector (for trains) or store as single value (for intersections)
            stringstream single_value(all_values);
            string part;
            while (getline(single_value, part, ',')) {
                data_map[item_name].push_back(part);
            }
        }
    }
}

// Prints and displays intersection and train data
void displayData(const unordered_map<string, vector<string>>& data, const string& title) {
    cout << title << ":\n";
    for (const auto& [key, values] : data) {
        cout << key << ": ";
        for (const auto& val : values) {
            cout << val << " ";
        }
        cout << endl;
    }
}

int main() {
    // Stores intersection data and train data
    unordered_map<string, vector<string>> intersections;
    unordered_map<string, vector<string>> trains;

    // Loads data from the files
    parseFile("intersections.txt", intersections);
    parseFile("trains.txt", trains);

    // Displays the data from the files
    displayData(intersections, "Intersections and Capacities");
    displayData(trains, "Trains and Routes");

    return 0;
}
/*
Group Number: G
Author: Eric Vo
Email: eric.t.vo@okstate.edu
Date: 13 April 2025
*/

#include "TrainFileParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/*
Reads and pareses the two text files (intersections.txt and trains.txt)

Input: filename - the name of the file to read
Output: data_map - populates reference map

Map contents:
    Key: Train name or an intersection (TrainA, TrainB... or IntersectionA, IntersectionB)
    Value: vector of strings (intersection capacities or route intersections) 
*/
void parseFile(const string& filename, map<string, vector<string>>& data_map) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        perror("Error details: ");
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

// Prints and displays intersection and train data (optional)
void displayData(const map<string, vector<string>>& data, const string& title) {
    cout << title << ":\n";
    for (const auto& [key, values] : data) {
        cout << key << ": ";
        for (const auto& val : values) {
            cout << val << " ";
        }
        cout << endl;
    }
}

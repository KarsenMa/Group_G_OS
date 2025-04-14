/*
Group Number: G
Author: Eric Vo
Email: eric.t.vo@okstate.edu
Date: 6 April 2025
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/**
 * parseFile:
 *   Reads the given file line by line.
 *   Each line is "key:value" or "key:value1,value2,..."
 *   We store them in a map<string, vector<string>>.
 */
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

            // Split the substring by ',' if present
            stringstream single_value(all_values);
            string part;
            while (getline(single_value, part, ',')) {
                data_map[item_name].push_back(part);
            }
        }
    }
}

void displayData(const unordered_map<string, vector<string>>& data, const string& title) {
    cout << title << ":\n";
    for (const auto& [key, values] : data) {
        cout << "  " << key << ": ";
        for (const auto& val : values) {
            cout << val << " ";
        }
        cout << endl;
    }
}

/**
 * testEricFileParsing:
 *   A helper function we can call from main() so that we do not
 *   introduce a second main(). This simply demonstrates Eric’s code.
 */
void testEricFileParsing() {
    cout << "\n--- Demonstrating Eric's file parsing ---\n";
    unordered_map<string, vector<string>> intersections;
    unordered_map<string, vector<string>> trains;

    // Filenames must exist in the same folder. Adjust as needed.
    parseFile("intersections.txt", intersections);
    parseFile("trains.txt", trains);

    displayData(intersections, "Intersections and Capacities");
    displayData(trains, "Trains and Routes");
}

/*
Group Number: G
Author: Eric Vo
Email: eric.t.vo@okstate.edu
Date: 13 April 2025
*/

#ifndef TRAINFILEPARSER_H
#define TRAINFILEPARSER_H

#include <string>
#include <map>
#include <vector>

// Function to parse a file into a map
void parseFile(const std::string& filename, std::map<std::string, std::vector<std::string>>& data_map);

// Function to display parsed data (optional)
void displayData(const std::map<std::string, std::vector<std::string>>& data, const std::string& title);

#endif

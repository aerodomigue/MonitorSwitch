#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

// Function to save peripheral IDs to a file
void savePeripheralIDs(const std::vector<std::string>& ids, const std::string& filename);

// Function to load peripheral IDs from a file
std::vector<std::string> loadPeripheralIDs(const std::string& filename);

// Function to validate a peripheral ID
bool validatePeripheralID(const std::string& id);

#endif // UTILS_H
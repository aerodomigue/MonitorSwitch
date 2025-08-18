#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>

bool savePeripheralID(const std::string& filePath, const std::string& peripheralID) {
    std::ofstream outFile(filePath);
    if (!outFile) {
        std::cerr << "Error opening file for writing: " << filePath << std::endl;
        return false;
    }
    outFile << peripheralID;
    outFile.close();
    return true;
}

bool loadPeripheralID(const std::string& filePath, std::string& peripheralID) {
    std::ifstream inFile(filePath);
    if (!inFile) {
        std::cerr << "Error opening file for reading: " << filePath << std::endl;
        return false;
    }
    std::getline(inFile, peripheralID);
    inFile.close();
    return true;
}
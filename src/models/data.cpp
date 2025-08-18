#include "data.h"

Data::Data() : peripheralID(-1) {}

Data::~Data() {}

void Data::setPeripheralID(int id) {
    peripheralID = id;
}

int Data::getPeripheralID() const {
    return peripheralID;
}
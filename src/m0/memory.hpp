#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <vector>
#include "DataCell.hpp"

void set_register(unsigned int reg, DataCell cell);

class address
{
public:
    bool reg;
    unsigned int index;
    address(bool reg, int index);
    DataCell* getCell();
};

#endif
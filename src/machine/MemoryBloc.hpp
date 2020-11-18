#pragma once

#include <vector>

#include "DataCell.hpp"

class MemoryBloc
{
public:
    std::vector<DataCell> cells;
    DataCell& operator[](const unsigned int i);
    DataCell& operator[](const Address& addr);
};
#pragma once

#include "DataCell.hpp"
#include <vector>

class ChoicePoint{
public:
    std::vector<DataCell> arg_registers;
    unsigned int CE;
    unsigned int CP;
    unsigned int B;
    unsigned int BP;
    unsigned int TR;
    unsigned int H;
};
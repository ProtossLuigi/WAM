#pragma once

#include "MemoryBloc.hpp"

class Enviroment
{
public:
    unsigned int E;
    unsigned int CP;
    std::vector<DataCell> perm_vars;
    Enviroment();
    Enviroment(unsigned int E, unsigned int CP);
};
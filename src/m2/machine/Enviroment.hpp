#pragma once

#include "MemoryBloc.hpp"

class Enviroment
{
public:
    unsigned int CP;
    MemoryBloc perm_vars;
    Enviroment(unsigned int CP);
};
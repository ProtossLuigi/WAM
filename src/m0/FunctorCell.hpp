#ifndef FUNCTORCELL_HPP
#define FUNCTORCELL_HPP

#include "DataCell.hpp"
#include <string>

class FunctorCell : public DataCell
{
public:
    std::string name;
    int subtcount;
    FunctorCell(std::string name, int subtcount);
};

#endif
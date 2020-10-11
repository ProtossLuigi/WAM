#include "DataCell.hpp"
#include <string>

class FunctorCell : DataCell
{
public:
    std::string name;
    int subtcount;
    FunctorCell(std::string name, int subtcount);
};

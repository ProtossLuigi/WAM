#ifndef REFCELL_HPP
#define REFCELL_HPP

#include "DataCell.hpp"

enum reftype {str, ref};

class RefCell : public DataCell
{
public:
    bool isRef = true;
    reftype type;
    DataCell* addr;
    RefCell(reftype type, DataCell* addr);
};

#endif
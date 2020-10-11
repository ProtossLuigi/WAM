#include "DataCell.hpp"

enum reftype {str, ref};

class RefCell : DataCell
{
public:
    reftype type;
    int index;
    RefCell(reftype type, int index);
};

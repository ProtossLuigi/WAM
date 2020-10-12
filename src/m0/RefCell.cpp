#include "RefCell.hpp"

RefCell::RefCell(reftype type, DataCell* addr){
    this->type = type;
    if(addr == nullptr){
        this->addr = this;
    } else{
        this->addr = addr;
    }
}
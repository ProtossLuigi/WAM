#include "MemoryBloc.hpp"
#include "Address.hpp"

DataCell& MemoryBloc::operator[](const unsigned int i){
    if(cells.size() <= i){
        cells.resize(i+1);
    }
    return cells[i];
}

DataCell& MemoryBloc::operator[](const Address& addr){
    if(addr.bloc != this){
        throw "Address points to diffrent area of memory.";
    }
    return this->operator[](addr.index);
}
#include "MemoryBloc.hpp"
#include "Address.hpp"

DataCell& MemoryBloc::operator[](const unsigned int i){
    while(cells.size() < i+1){
        cells.emplace_back("REF", Address(*this, cells.size()));
    }
    return cells[i];
}

DataCell& MemoryBloc::operator[](const Address& addr){
    if(addr.bloc != this){
        throw "Address points to diffrent area of memory.\n";
    }
    return this->operator[](addr.index);
}
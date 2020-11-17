#include "Address.hpp"
#include "MemoryBloc.hpp"

Address::Address(MemoryBloc& bloc, unsigned int index){
    this->bloc = &bloc;
    this->index = index;
}

DataCell& Address::getCell(){
    return (*bloc)[index];
}

Address& Address::operator+=(int offset){
    index += offset;
    return *this;
}

bool Address::operator==(const Address& addr){
    return (this->bloc == addr.bloc) && (this->index == addr.index);
}

bool Address::operator!=(const Address& addr){
    return !this->operator==(addr);
}

Address operator+(Address addr, int offset){
    addr += offset;
    return addr;
}
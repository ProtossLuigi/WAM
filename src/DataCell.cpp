#include "DataCell.hpp"
#include "Address.hpp"

DataCell::DataCell(const std::string tag, Address addr){
    this->tag = tag;
    this->addr = std::shared_ptr<Address>(new Address(addr));
}

DataCell::DataCell(const std::string tag){
    this->tag = tag;
}

void DataCell::setAddr(Address addr){
    this->addr.reset(new Address(addr));
}

Address DataCell::getAddr(){
    if(tag != "REF" && tag != "STR"){
        throw "Cannot dereference functor cell.";
    }
    return *addr;
}
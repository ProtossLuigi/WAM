#include "DataCell.hpp"
#include "Address.hpp"

DataCell::DataCell(){
    this->tag = "";
}

DataCell::DataCell(const std::string tag, Address addr){
    this->tag = tag;
    this->addr = std::shared_ptr<Address>(&addr);
}

DataCell::DataCell(const std::string tag){
    this->tag = tag;
}

void DataCell::setAddr(Address addr){
    this->addr.reset(&addr);
}

Address DataCell::getAddr(){
    return *addr;
}
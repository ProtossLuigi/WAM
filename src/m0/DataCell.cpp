#include "DataCell.hpp"
#include "Address.hpp"

DataCell::DataCell(){
    this->tag = "";
}

DataCell::DataCell(const std::string tag, const Address& addr){
    this->tag = tag;
    this->addr = addr;
}

DataCell::DataCell(const std::string tag){
    this->tag = tag;
}
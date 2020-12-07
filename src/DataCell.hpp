#pragma once

#include <string>
#include <memory>

#include "Address.hpp"

class DataCell
{
private:
    std::shared_ptr<Address> addr;
public:
    std::string tag;
    DataCell(const std::string tag, Address addr);
    DataCell(const std::string tag);
    void setAddr(Address addr);
    Address getAddr();
};
#pragma once

#include <string>
#include <memory>

#include "Address.hpp"

class DataCell
{
public:
    std::string tag;
    std::shared_ptr<Address> addr;
    DataCell();
    DataCell(const std::string tag, Address addr);
    DataCell(const std::string tag);
};
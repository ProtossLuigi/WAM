#pragma once

#include <string>

#include "Address.hpp"

class DataCell
{
public:
    std::string tag;
    Address addr;
    DataCell();
    DataCell(const std::string tag, const Address& addr);
    DataCell(const std::string tag);
};
#pragma once

#include <memory>

class MemoryBloc;
class DataCell;

class Address
{
public:
    MemoryBloc* bloc;
    unsigned int index;
    Address() = default;
    Address(MemoryBloc& bloc, unsigned int index);
    DataCell& getCell();
    Address& operator+=(int offset);
    bool operator==(const Address& addr);
    bool operator!=(const Address& addr);
};

Address operator+(Address addr, int offset);
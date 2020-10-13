#pragma once

class MemoryBloc;
class DataCell;

class Address
{
public:
    MemoryBloc* bloc;
    unsigned int index;
    Address();
    Address(MemoryBloc& bloc, unsigned int index);
    DataCell& getCell();
    Address& operator+=(int offset);
    bool operator==(const Address& addr);
    bool operator!=(const Address& addr);
};

Address operator+(Address addr, int offset);
#include "memory.hpp"

std::vector<DataCell> heap;
std::vector<DataCell> registers;
int H = 0;

address::address(bool reg, int index){
    this->reg = reg;
    this->index = index;
}

DataCell* address::getCell(){
    if(reg){
        return &registers[index];
    } else{
        return &heap[index];
    }
}

void set_register(unsigned int reg, DataCell cell){
    if(registers.size() < reg){
        registers.resize(reg + 1);
        registers[reg] = cell;
    } else if(registers.size() == reg){
        registers.push_back(cell);
    } else{
        registers[reg] = cell;
    }
}
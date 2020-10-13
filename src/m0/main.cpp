#include <iostream>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include "Address.hpp"
#include "DataCell.hpp"
#include "MemoryBloc.hpp"

enum memory_mode {write, read};

MemoryBloc heap;
MemoryBloc registers;
Address H (heap, 0);
memory_mode mode;

DataCell string_to_functor(std::string str){
    return DataCell(str);
}

void put_structure(std::string functor, int reg){
    heap[H] = DataCell("STR", H+1);
    heap[H+1] = string_to_functor(functor);
    registers[reg] = heap[H];   //pass by value/reference???
    H += 2;
}

void set_variable(int reg){
    heap[H] = DataCell("REF", H);
    registers[reg] = heap[H];
    H += 1;
}

void set_value(int reg){
    heap[H] = registers[reg];
    H += 1;
}

Address deref(const Address& addr){
    if(heap[addr].tag == "REF" && heap[addr].addr != addr){
        return deref(heap[addr].addr);
    }
    return addr;
}

void bind(const Address& a, const Address& b){

}

int get_structure(std::string functor, int reg){
    Address addr = deref(registers[reg].addr);
    DataCell& cell = addr.getCell();
    if(cell.tag == "REF"){
        heap[H] = DataCell("STR", H+1);
            heap[H+1] = string_to_functor(functor);
            bind(addr, H);
            H += 2;
            mode = write;
    } else if(cell.tag == "STR"){

    }else{
        return 1;
    }
    return 0;
}

void unify_variable(int reg){
    //TODO
}

void unify_value(int reg){
    //TODO
}

int read_query(std::string q){
    std::ifstream f;
    f.open(q, std::ifstream::in);
    std::vector<std::regex> vr;
    vr.emplace_back("put_structure\\((\\w+\\/\\d+),(\\d+)\\)$");
    vr.emplace_back("set_variable\\((\\d+)\\)$");
    vr.emplace_back("set_value\\((\\d+)\\)$");
    std::string line;
    std::smatch match;
    while(getline(f, line)){
        unsigned int i = 0;
        while(i < vr.size() && !std::regex_match(line, match, vr[i])){
            i++;
        }
        switch (i)
        {
        case 0:
            put_structure(match[1], std::stoi(match[2]));
            break;
        case 1:
            set_variable(std::stoi(match[1]));
            break;
        case 2:
            set_value(std::stoi(match[1]));
            break;
        default:
            f.close();
            return 2;
        }
    }
    f.close();
    return 0;
}

int read_program(std::string p){
    std::ifstream f;
    f.open(p, std::ifstream::in);
    std::vector<std::regex> vr;
    vr.emplace_back("get_structure\\((\\w+\\/\\d+),(\\d+)\\)$");
    vr.emplace_back("unify_variable\\((\\d+)\\)$");
    vr.emplace_back("unify_value\\((\\d+)\\)$");
    std::string line;
    std::smatch match;
    while(getline(f, line)){
        unsigned int i = 0;
        while(i < vr.size() && !std::regex_match(line, match, vr[i])){
            i++;
        }
        switch (i)
        {
        case 0:
            if(get_structure(match[1], std::stoi(match[2]))){
                f.close();
                return 1;
            }
            break;
        case 1:
            unify_variable(std::stoi(match[1]));
            break;
        case 2:
            unify_value(std::stoi(match[1]));
            break;
        default:
            f.close();
            return 2;
        }
    }
    f.close();
    return 0;
}

int main(int argc, char** argv){
    std::string query = "";
    std::string program = "";
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-q") == 0 && i+1 < argc){
            query = std::string(argv[i+1]);
        } else if(strcmp(argv[i], "-p") == 0 && i+1 < argc){
            program = std::string(argv[i+1]);
        }
    }
    if(query == "" || program == ""){
        if(query == ""){
            std::cout << "No input query.\n";
        }
        if(program == ""){
            std::cout << "No input program.\n";
        }
        return 1;
    }
    read_query(query);

    return 0;
}
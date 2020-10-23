#include <iostream>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include "Address.hpp"
#include "DataCell.hpp"
#include "MemoryBloc.hpp"
#include <stack>

typedef std::pair<std::string,unsigned int> functor;

enum memory_mode {write, read};

MemoryBloc heap;
MemoryBloc registers;
Address H (heap, 0);
Address S;
memory_mode mode;

void print_memory(){
    std::cout << "HEAP:\n";
    for(DataCell cell : heap.cells){
        if(cell.tag == "REF" || cell.tag == "STR"){
            std::cout << "< " << cell.tag << ", ";
            if(cell.getAddr().bloc == &heap){
                std::cout << "heap[";
            } else if(cell.getAddr().bloc == &registers){
                std::cout << "registers[";
            }
            std::cout << cell.getAddr().index << "] >\n";
        } else{
            std::cout << "< " << cell.tag << " >\n";
        }
    }
    std::cout << "\nregisters:\n";
    for(DataCell cell : registers.cells){
        if(cell.tag == "REF" || cell.tag == "STR"){
            std::cout << "< " << cell.tag << ", ";
            if(cell.getAddr().bloc == &heap){
                std::cout << "heap[";
            } else if(cell.getAddr().bloc == &registers){
                std::cout << "registers[";
            }
            std::cout << cell.getAddr().index << "] >\n";
        } else{
            std::cout << "< " << cell.tag << " >\n";
        }
    }
}

DataCell string_to_functor(std::string str){
    return DataCell(str);
}

functor get_functor(std::string str){
    std::smatch match;
    if(std::regex_match(str, match, std::regex("(w+)\\/(d+)"))){
        return functor(match[1],std::stoi(match[2]));
    } else{
        throw "Argument not a representation of a functor.";
    }
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
    if(heap[addr].tag == "REF" && heap[addr].getAddr() != addr){
        return deref(heap[addr].getAddr());
    }
    return addr;
}

void bind(Address& a, Address& b){
    if(a.getCell().getAddr() == a){
        a.getCell().setAddr(b);
    } else{
        b.getCell().setAddr(a);
    }
}

int unify(Address a1, Address a2){
    std::stack<Address> pdl;
    pdl.push(a1);
    pdl.push(a2);
    while(!pdl.empty()){
        Address d1 = deref(pdl.top());
        pdl.pop();
        Address d2 = deref(pdl.top());
        pdl.pop();
        if(d1 != d2){
            if(d1.getCell().tag == "REF" || d2.getCell().tag == "REF"){
                bind(d1,d2);
            } else{
                functor f1 = get_functor(d1.getCell().getAddr().getCell().tag), f2 = get_functor(d2.getCell().getAddr().getCell().tag);
                if(f1.first == f2.first && f1.second == f2.second){
                    for(unsigned int i = 1; i <= f1.second; i++){
                        pdl.push(d1.getCell().getAddr()+i);
                        pdl.push(d2.getCell().getAddr()+i);
                    }
                } else{
                    return 1;
                }
            }
        }
    }
    return 0;
}

int get_structure(std::string functor, int reg){
    Address addr = deref(Address(registers, reg));
    DataCell& cell = addr.getCell();
    if(cell.tag == "REF"){
        heap[H] = DataCell("STR", H+1);
            heap[H+1] = string_to_functor(functor);
            bind(addr, H);
            H += 2;
            mode = write;
    } else if(cell.tag == "STR"){
        if(heap[cell.getAddr()].tag == functor){
            S = cell.getAddr() + 1;
            mode = read;
        } else{
            return 1;
        }
    }else{
        return 1;
    }
    return 0;
}

void unify_variable(int reg){
    switch (mode)
    {
    case read:
        registers[reg] = heap[S];
        break;
    case write:
        heap[H] = DataCell("REF", H);
        registers[reg] = heap[H];
        H += 1;
        break;
    default:
        break;
    }
    S += 1;
}

void unify_value(int reg){
    switch (mode)
    {
    case read:
        unify(Address(registers, reg), S);
        break;
    case write:
        heap[H] = registers[reg];
        H += 1;
    default:
        break;
    }
    S += 1;
}

int read_query(std::string q){
    std::ifstream f;
    f.open(q, std::ifstream::in);
    std::vector<std::regex> vr;
    vr.emplace_back("put_structure (\\w+\\/\\d+),X(\\d+)$");
    vr.emplace_back("set_variable X(\\d+)$");
    vr.emplace_back("set_value X(\\d+)$");
    std::string line;
    std::smatch match;
    int line_no = 1;
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
            std::cout << "Error in line " << line_no << "\n";
            f.close();
            return 2;
        }
        line_no++;
    }
    f.close();
    return 0;
}

int read_program(std::string p){
    std::ifstream f;
    f.open(p, std::ifstream::in);
    std::vector<std::regex> vr;
    vr.emplace_back("get_structure (\\w+\\/\\d+),X(\\d+)$");
    vr.emplace_back("unify_variable X(\\d+)$");
    vr.emplace_back("unify_value X(\\d+)$");
    std::string line;
    std::smatch match;
    int line_no = 1;
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
            std::cout << "Error in line " << line_no << "\n";
            f.close();
            return 2;
        }
        line_no++;
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
    try{
        read_query(query);
        print_memory();
        int result = read_program(program);
        print_memory();
        switch (result)
        {
        case 0:
            break;
        case 1:
            std::cout << "Can't unify.\n";
            break;
        case 2:
            std::cout << "Critical error.\n";
            break;
        default:
            break;
        }
    } catch(char const* str){
        std::cout << str;
        return 1;
    }
    return 0;
}
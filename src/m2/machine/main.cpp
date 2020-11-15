#include <iostream>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include "Address.hpp"
#include "DataCell.hpp"
#include "MemoryBloc.hpp"
#include "Enviroment.hpp"
#include <stack>
#include <unordered_map>

typedef std::pair<std::string,unsigned int> functor;

enum memory_mode {write, read};

MemoryBloc heap;
MemoryBloc registers;
MemoryBloc arg_registers;
MemoryBloc perm_registers;
std::vector<std::vector<std::string>> code;
std::stack<Enviroment> enviroments;
std::unordered_map<std::string, unsigned int> labels;
Address H (heap, 0);
Address S;
memory_mode mode;
unsigned int P = 0;
unsigned int CP;

std::string address_to_string(Address addr){
    if(addr.bloc == &heap){
            return "H" + std::to_string(addr.index);
        } else if(addr.bloc == &registers){
            return "X" + std::to_string(addr.index);
        } else if(addr.bloc == &arg_registers){
            return "A" + std::to_string(addr.index);
        } else{
            throw "Error: Invalid address.\n";
        }
        
}

void print_memory(){
    std::cout << "HEAP:\n";
    for(DataCell cell : heap.cells){
        if(cell.tag == "REF" || cell.tag == "STR"){
            std::cout << "< " << cell.tag << ", ";
            if(cell.getAddr().bloc == &heap){
                std::cout << "H";
            } else if(cell.getAddr().bloc == &registers){
                std::cout << "X";
            } else if(cell.getAddr().bloc == &arg_registers){
                std::cout << "A";
            }
            std::cout << cell.getAddr().index << " >\n";
        } else{
            std::cout << "< " << cell.tag << " >\n";
        }
    }
    std::cout << "\nREGISTERS:\n";
    for(DataCell cell : registers.cells){
        if(cell.tag == "REF" || cell.tag == "STR"){
            std::cout << "< " << cell.tag << ", ";
            if(cell.getAddr().bloc == &heap){
                std::cout << "H";
            } else if(cell.getAddr().bloc == &registers){
                std::cout << "X";
            } else if(cell.getAddr().bloc == &arg_registers){
                std::cout << "A";
            }
            std::cout << cell.getAddr().index << " >\n";
        } else{
            std::cout << "< " << cell.tag << " >\n";
        }
    }
    std::cout << "\nARGUMENT REGISTERS:\n";
    for(DataCell cell : arg_registers.cells){
        if(cell.tag == "REF" || cell.tag == "STR"){
            std::cout << "< " << cell.tag << ", ";
            if(cell.getAddr().bloc == &heap){
                std::cout << "H";
            } else if(cell.getAddr().bloc == &registers){
                std::cout << "X";
            } else if(cell.getAddr().bloc == &arg_registers){
                std::cout << "A";
            }
            std::cout << cell.getAddr().index << " >\n";
        } else{
            std::cout << "< " << cell.tag << " >\n";
        }
    }
}

std::pair<std::string, unsigned int> get_functor(std::string str){
    std::smatch match;
    if(std::regex_match(str, match, std::regex("(\\w+)\\/(\\d+)"))){
        return std::pair<std::string, int>(match[1], std::stoi(match[2]));
    } else{
        throw "Error: Argument " + str + " is not a representation of a functor.\n";
    }
}

Address deref(Address addr){
    if(addr.getCell().tag == "REF" && addr.getCell().getAddr() != addr){
        return deref(addr.getCell().getAddr());
    }
    return addr;
}

std::string term_to_string(Address addr){
    DataCell cell = deref(addr).getCell();
    if (cell.tag == "REF")
    {
        return "_" + address_to_string(cell.getAddr());
    } else if (cell.tag == "STR")
    {
        functor f = get_functor(cell.getAddr().getCell().tag);
        std::string str = f.first;
        if (f.second > 0)
        {
            str += "(";
            for (size_t i = 1; i < f.second+1; i++)
            {
                if (i > 1)
                {
                    str += ",";
                }
                str += term_to_string(cell.getAddr()+i);
            }
            str += ")";
        }
        return str;
    } else
    {
        return cell.tag;
    }
}

DataCell string_to_functor(std::string str){
    return DataCell(str);
}

Address string_to_address(std::string str){
    std::smatch match;
    if(std::regex_match(str, match, std::regex("(\\w)(\\d+)"))){
        if (match[1] == "H")
        {
            return Address(heap, std::stoi(match[2]));
        } else if (match[1] == "X")
        {
            return Address(registers, std::stoi(match[2]));
        } else if (match[1] == "A")
        {
            return Address(arg_registers, std::stoi(match[2]));
        } else if (match[1] == "Y")
        {
            return Address(perm_registers, std::stoi(match[2]));
        }
    }
    throw str + " is not a correct representation of an address.\n";
}

void bind(Address& a, Address& b){
    if(a.getCell().getAddr() == a){
        a.getCell().setAddr(b);
    } else{
        b.getCell().setAddr(a);
    }
}

bool unify(Address a1, Address a2){
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
                    return true;
                }
            }
        }
    }
    return false;
}

// Machine instructions

void put_structure(std::string functor, Address reg){
    H.getCell() = DataCell("STR", H+1);
    (H+1).getCell() = string_to_functor(functor);
    reg.getCell() = H.getCell();
    H += 2;
}

void set_variable(Address reg){
    H.getCell() = DataCell("REF", H);
    reg.getCell() = H.getCell();
    H += 1;
}

void set_value(Address reg){
    H.getCell() = reg.getCell();
    H += 1;
}

bool get_structure(std::string functor, Address reg){
    Address addr = deref(reg);
    DataCell& cell = addr.getCell();
    if(cell.tag == "REF"){
        H.getCell() = DataCell("STR", H+1);
        (H+1).getCell() = string_to_functor(functor);
        bind(addr, H);
        H += 2;
        mode = memory_mode(0);
    } else if(cell.tag == "STR"){
        if(cell.getAddr().getCell().tag == functor){
            S = cell.getAddr() + 1;
            mode = read;
        } else{
            return true;
        }
    }else{
        throw "Error: Address points to functor cell. Expected variable cell.";
    }
    return false;
}

void unify_variable(Address reg){
    switch (mode)
    {
    case read:
        reg.getCell() = S.getCell();
        break;
    case 0:
        H.getCell() = DataCell("REF", H);
        reg.getCell() = H.getCell();
        H += 1;
        break;
    default:
        break;
    }
    S += 1;
}

bool unify_value(Address reg){
    switch (mode)
    {
    case read:
        if(unify(reg, S)){
            return true;
        }
        break;
    case 0:
        H.getCell() = reg.getCell();
        H += 1;
    default:
        break;
    }
    S += 1;
    return false;
}

void put_variable(Address reg, Address arg_reg){
    H.getCell() = DataCell("REF", H);
    reg.getCell() = H.getCell();
    arg_reg.getCell() = H.getCell();
    H += 1;
}

void put_value(Address reg, Address arg_reg){
    arg_reg.getCell() = reg.getCell();
}

void get_variable(Address reg, Address arg_reg){
    reg.getCell() = arg_reg.getCell();
}

bool get_value(Address reg, Address arg_reg){
    return unify(reg, arg_reg);
}

bool call(std::string str){
    CP = P;
    auto pos = labels.find(str);
    if (pos == labels.end())
    {
        return true;
    } else
    {
        P = pos->second - 1;
    }
    return false;
}

void proceed(){
    P = CP;
}

void allocate(int n){
    if(!enviroments.empty()){
        enviroments.top().perm_vars = perm_registers;
    }
    Enviroment new_env = Enviroment(CP);
    perm_registers = new_env.perm_vars;
    enviroments.push(new_env);
}

void deallocate(){
    P = enviroments.top().CP;
    enviroments.pop();
    perm_registers = enviroments.top().perm_vars;
}

void wam_write(){
    std::cout << term_to_string(Address(arg_registers, 0)) << std::endl;
}

void nl(){
    std::cout << std::endl;
}

// Machine instructions end

void load_builtin_predicates(){

    labels["write/1"] = code.size();
    code.push_back(std::vector<std::string> {"write"});
    code.push_back(std::vector<std::string> {"proceed"});

    labels["nl/0"] = code.size();
    code.push_back(std::vector<std::string> {"nl"});
    code.push_back(std::vector<std::string> {"proceed"});
}

void load_program(std::string filename){
    load_builtin_predicates();
    std::ifstream f;
    f.open(filename, std::ifstream::in);
    std::string line;
    std::smatch match;
    std::regex r_label("[ \\t]*(\\w+\\/\\d+)[ \\t]*\\:$");
    std::regex r_instr("[ \\t]*([a-z]\\w*)([ \\t]+(\\w+|\\w+\\/\\d+)([ \\t]*,[ \\t]*(\\w+|\\w+\\/\\d+))*)?$");
    int line_no = 1;
    while (getline(f, line))
    {
        if (std::regex_match(line, match, r_label))
        {
            if (labels.find(match[1]) == labels.end())
            {
                labels[match[1]] = code.size();
            } else
            {
                throw "Error: Duplicate label " + std::string(match[1]) + ".\n";
            }
        } else if (std::regex_match(line, match, r_instr))
        {
            std::vector<std::string> code_line;
            for (size_t i = 1; i < match.size(); i += 2)
            {
                code_line.push_back(match[i]);
            }
            code.push_back(code_line);
        } else if (line != "")
        {
            std::cerr << "Warning: Unrecognized code at line " << line_no << std::endl;
        }
        line_no++;
    }
    f.close();
}

void load_query(std::string filename){
    P = code.size();
    std::ifstream f;
    f.open(filename, std::ifstream::in);
    std::string line;
    std::smatch match;
    std::regex r_instr("[ \\t]*([a-z]\\w*)([ \\t]+(\\w+|\\w+\\/\\d+)([ \\t]*,[ \\t]*(\\w+|\\w+\\/\\d+))*)?$");
    int line_no = 1;
    while (getline(f, line))
    {
        if (std::regex_match(line, match, r_instr))
        {
            std::vector<std::string> code_line;
            for (size_t i = 1; i < match.size(); i += 2)
            {
                code_line.push_back(match[i]);
            }
            code.push_back(code_line);
        } else if (line != "")
        {
            std::cerr << "Warning: Unrecognized code at line " << line_no << std::endl;
        }
        line_no++;
    }
    f.close();
}

void run(){
    bool fail = false;
    while (P < code.size())
    {
        std::vector<std::string> command = code[P];
        if (command[0] == "put_structure")
        {
            put_structure(command[1], string_to_address(command[2]));
        } else if (command[0] == "set_variable")
        {
            set_variable(string_to_address(command[1]));
        } else if (command[0] == "set_value")
        {
            set_value(string_to_address(command[1]));
        } else if (command[0] == "get_structure")
        {
            fail = get_structure(command[1], string_to_address(command[2]));
        } else if (command[0] == "unify_variable")
        {
            unify_variable(string_to_address(command[1]));
        } else if (command[0] == "unify_value")
        {
            fail = unify_value(string_to_address(command[1]));
        } else if (command[0] == "put_variable")
        {
            put_variable(string_to_address(command[1]), string_to_address(command[2]));
        } else if (command[0] == "put_value")
        {
            put_value(string_to_address(command[1]), string_to_address(command[2]));
        } else if (command[0] == "get_variable")
        {
            get_variable(string_to_address(command[1]), string_to_address(command[2]));
        } else if (command[0] == "get_value")
        {
            fail = get_value(string_to_address(command[1]), string_to_address(command[2]));
        } else if (command[0] == "call")
        {
            fail = call(command[1]);
        } else if (command[0] == "proceed")
        {
            proceed();
        } else if (command[0] == "allocate")
        {
            allocate(std::stoi(command[1]));
        } else if (command[0] == "deallocate")
        {
            deallocate();
        } else if (command[0] == "write")
        {
            wam_write();
        } else if (command[0] == "nl")
        {
            nl();
        } else
        {
            std::cerr << "Warning: Unrecognized instruction " << command[0] << " in memory at line " << P <<". Skipping.\n";
        }
        if(fail){
            break;
        }
        P++;
    }
    if(fail){
        std::cout << "false.\n";
    } else{
        std::cout << "true.\n";
    }
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
    if(query == ""){
        if(query == ""){
            std::cout << "No input query.\n";
        }
        return 1;
    }
    try{
        load_program(program);
        load_query(query);
        run();
        // print_memory();
    } catch(char const* str){
        std::cerr << str;
        return 1;
    }
    return 0;
}
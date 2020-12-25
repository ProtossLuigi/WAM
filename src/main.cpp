#include <iostream>
#include <vector>
#include <memory>
#include <regex>
#include <fstream>
#include "Address.hpp"
#include "DataCell.hpp"
#include "MemoryBloc.hpp"
#include "Enviroment.hpp"
#include "ChoicePoint.hpp"
#include <stack>
#include <unordered_map>
#include <chrono>
#include "../compiler_y.hpp"

using namespace std::chrono;

extern int compile(std::string in_string, std::string& out_string);

typedef std::pair<std::string,unsigned int> functor;

enum memory_mode {write, read};

unsigned int code_size;

MemoryBloc heap;
MemoryBloc registers;
MemoryBloc arg_registers;
MemoryBloc perm_registers;
std::vector<std::vector<std::string>> code;
std::vector<Enviroment> and_stack;
std::vector<ChoicePoint> or_stack;
std::unordered_map<std::string, unsigned int> labels;
std::vector<Address> trail;
Address H (heap, 0);
Address S;
memory_mode mode;
unsigned int P = 0;
unsigned int CP;
int E = -1;
int B = -1;
unsigned int num_of_args;
unsigned int HB = 0;

bool stats = false;
unsigned int inferences = 0;
high_resolution_clock::time_point last_time;
duration<double> timer = duration<double>::zero();

std::string term_to_string(Address addr);

std::string address_to_string(Address addr){
    if(addr.bloc == &heap){
            return "H" + std::to_string(addr.index);
        } else if(addr.bloc == &registers){
            return "X" + std::to_string(addr.index);
        } else if(addr.bloc == &arg_registers){
            return "A" + std::to_string(addr.index);
        }else if(addr.bloc == &perm_registers){
            return "Y" + std::to_string(addr.index);
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
    if(std::regex_match(str, match, std::regex("([\\w\\.\\[\\]]+)\\/(\\d+)"))){
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

bool is_list(Address addr){
    DataCell cell = deref(addr).getCell();
    if(cell.tag == "STR"){
        if (heap[cell.getAddr()].tag == "[]/0")
        {
            return true;
        } else if (heap[cell.getAddr()].tag == "./2")
        {
            return is_list(cell.getAddr()+2);
        } else
        {
            return false;
        }
    }
    return false;
}

std::string list_to_string(Address addr){
    addr = deref(addr).getCell().getAddr();
    std::string str = "";
    while (addr.getCell().tag != "[]/0")
    {
        if (str != "")
        {
            str += ",";
        }
        str += term_to_string(addr+1);
        addr = deref(addr+2).getCell().getAddr();
    }
    return str;
}

std::string term_to_string(Address addr){
    DataCell cell = deref(addr).getCell();
    if (cell.tag == "REF")
    {
        return "_" + address_to_string(cell.getAddr());
    } else if (cell.tag == "STR")
    {
        if (is_list(addr))
        {
            std::string str = "[";
            str += list_to_string(addr);
            str += "]";
            return str;
        }
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
        a.getCell() = b.getCell();
        trail.push_back(a);
    } else{
        b.getCell() = a.getCell();
        trail.push_back(b);
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

bool backtrack(){
    if(or_stack.empty()){
        return true;
    } else{
        P = or_stack.back().BP;
        return false;
    }
}

void unwind_trail(unsigned int trail_end){
    while(trail.size() > trail_end){
        if(trail.back().index < HB){
            trail.back().getCell() = DataCell("REF", trail.back());
        }
        trail.pop_back();
    }
}

void switch_enviroment(int newE){
    if(E > -1){
        and_stack[E].perm_vars = perm_registers.cells;
    }
    if(newE > -1){
        perm_registers.cells = and_stack[newE].perm_vars;
    }
    E = newE;
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
    if (stats)
    {
        inferences++;
    }
    CP = P;
    std::smatch match;
    std::regex header("(\\w+|=)\\/(\\d+)");
    if(std::regex_match(str, match, header)){
        num_of_args = std::stoi(match[2]);
    }
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
    Enviroment newEnv(E, CP);
    int newE;
    if(E > B){
        newE = E+1;
    } else{
        newE = B+1;
    }
    if((int)and_stack.size() <= newE)
    {
        and_stack.emplace_back();
    }
    and_stack[newE] = newEnv;
    switch_enviroment(newE);
}

void deallocate(){
    P = and_stack[E].CP;
    switch_enviroment(and_stack[E].E);
}

void try_me_else(std::string label){
    or_stack.emplace_back();
    for (unsigned int i = 0; i < num_of_args; i++)
    {
        or_stack.back().arg_registers.push_back(arg_registers[i]);
    }
    or_stack.back().CE = E;
    or_stack.back().CP = CP;
    or_stack.back().B = B;
    or_stack.back().BP = labels.find(label)->second;
    or_stack.back().TR = trail.size();
    or_stack.back().H = H.index;
    B = E;
    HB = H.index;
}

void retry_me_else(std::string label){
    arg_registers.cells = or_stack.back().arg_registers;
    switch_enviroment(or_stack.back().CE);
    CP = or_stack.back().CP;
    or_stack.back().BP = labels.find(label)->second;
    unwind_trail(or_stack.back().TR);
    H.index = or_stack.back().H;
    HB = H.index;
}

void trust_me(){
    arg_registers.cells = or_stack.back().arg_registers;
    switch_enviroment(or_stack.back().CE);
    CP = or_stack.back().CP;
    unwind_trail(or_stack.back().TR);
    H.index = or_stack.back().H;
    B = or_stack.back().B;
    or_stack.pop_back();
    HB = or_stack.back().H;
}

void wam_write(){
    std::cout << term_to_string(Address(arg_registers, 0));
}

void nl(){
    std::cout << std::endl;
}

// Machine instructions end

void load_builtin_predicates(){
    labels["=/2"] = code.size();
    code.push_back(std::vector<std::string> {"get_variable", "X0", "A0"});
    code.push_back(std::vector<std::string> {"get_value", "X0", "A1"});
    code.push_back(std::vector<std::string> {"proceed"});

    labels["write/1"] = code.size();
    code.push_back(std::vector<std::string> {"write"});
    code.push_back(std::vector<std::string> {"proceed"});

    labels["nl/0"] = code.size();
    code.push_back(std::vector<std::string> {"nl"});
    code.push_back(std::vector<std::string> {"proceed"});

    labels["./2"] = code.size();
    code.push_back(std::vector<std::string> {"try_me_else", "DL0"});
    code.push_back(std::vector<std::string> {"get_structure", "[]/0", "A1"});
    code.push_back(std::vector<std::string> {"proceed"});
    labels["DL0"] = code.size();
    code.push_back(std::vector<std::string> {"trust_me"});
    code.push_back(std::vector<std::string> {"allocate", "1"});
    code.push_back(std::vector<std::string> {"get_variable", "Y0", "A1"});
    code.push_back(std::vector<std::string> {"put_variable", "Y0", "A0"});
    code.push_back(std::vector<std::string> {"put_structure", "./2", "A1"});
    code.push_back(std::vector<std::string> {"set_variable", "X0"});
    code.push_back(std::vector<std::string> {"set_variable", "X1"});
    code.push_back(std::vector<std::string> {"call", "=/2"});
    code.push_back(std::vector<std::string> {"deallocate"});
}

void load_program(std::iostream& f){
    load_builtin_predicates();
    std::string line;
    std::smatch match;
    std::regex r_label("[ \\t]*([\\w\\/\\.]+)[ \\t]*\\:$");
    std::regex r_instr("[ \\t]*([a-z]\\w*)([ \\t]+([\\w=\\/\\[\\]\\.]+)([ \\t]*,[ \\t]*([[:alnum:]=\\/\\[\\]\\.]+))*)?$");
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
            std::cerr << "Warning: Unrecognized program code at line " << line_no << std::endl;
        }
        line_no++;
    }
    code_size = code.size();
}

void load_query(std::iostream& f){
    P = code.size();
    std::string line;
    std::smatch match;
    std::regex r_instr("[ \\t]*([a-z]\\w*)([ \\t]+([\\w=\\/\\[\\]\\.]+)([ \\t]*,[ \\t]*([[:alnum:]=\\/\\[\\]\\.]+))*)?$");
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
            std::cerr << "Warning: Unrecognized query code at line " << line_no << std::endl;
        }
        line_no++;
    }
}

void run(){
    bool fail = false;
    while (true)
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
        } else if (command[0] == "try_me_else")
        {
            try_me_else(command[1]);
        } else if (command[0] == "retry_me_else")
        {
            retry_me_else(command[1]);
        } else if (command[0] == "trust_me")
        {
            trust_me();
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
        P++;
        if(P == code.size() && !fail){
            if (stats)
            {
                timer += high_resolution_clock::now() - last_time;
            }
            std::cout << "true\n" << "continue evaluation? (y/n): ";
            char c;
            std::cin >> c;
            while(c != 'y' && c != 'n')
            {
                std::cin >> c;
            }
            if (stats)
            {
                last_time = high_resolution_clock::now();
            }
            if(c == 'y')
            {
                fail = true;
            } else if (c == 'n')
            {
                return;
            }
            
        }
        if(fail){
            if(backtrack())
            {
                std::cout << "false.\n";
                return;
            } else
            {
                fail = false;
            }
        }
    }
}

void reset_memory(){
    code.resize(code_size);
    heap = MemoryBloc();
    registers = MemoryBloc();
    arg_registers = MemoryBloc();
    perm_registers = MemoryBloc();
    and_stack = std::vector<Enviroment>();
    or_stack = std::vector<ChoicePoint>();
    trail = std::vector<Address>();
    H = Address(heap, 0);
    P = 0;
    CP = code_size;
    E = -1;
    B = -1;
    HB = 0;
}

std::string string_from_file(const std::string& filename){
    std::ifstream ifs;
    std::stringstream ss;
    ifs.open(filename, std::ios_base::in);
    ss << ifs.rdbuf();
    ifs.close();
    return ss.str();
}

void reset_stats(){
    timer = duration<double>::zero();
    inferences = 0;
}

int only_compile(std::string filename_in, std::string filename_out){
    std::string si = string_from_file(filename_in), so;
    if (compile(si, so) > 0)
    {
        std::cerr << "Error: Compilation error.\n";
        return 1;
    }
    std::fstream fs_out;
    if (filename_out == "")
    {
        fs_out.open("a.out", std::fstream::out);
    } else
    {
        fs_out.open(filename_out, std::fstream::out);
    }
    fs_out << so;
    fs_out.close();
    return 0;
}

int only_run(std::string program_filename, std::string query_filename){
    std::fstream pfs, qfs;
    pfs.open(program_filename, std::fstream::in);
    qfs.open(query_filename, std::fstream::out);
    try
    {
        load_program(pfs);
        load_query(qfs);
        if (stats)
        {
            last_time = high_resolution_clock::now();
        }
        run();
        if (stats)
        {
            timer += high_resolution_clock::now() - last_time;
            std::cout << inferences << " inferences, " << timer.count() << " seconds\n";
        }
    }
    catch(char const* e)
    {
        std::cerr << e << '\n';
        pfs.close();
        qfs.close();
        return 1;
    }
    catch(const std::string& e)
    {
        std::cerr << e << '\n';
        pfs.close();
        qfs.close();
        return 1;
    }
    pfs.close();
    qfs.close();
    return 0;
}

int compile_and_run(std::string program_filename){
    if (program_filename == "")
    {
        std::stringstream pss;
        load_program(pss);
    } else
    {
        std::string pis = string_from_file(program_filename), pos;
        if (compile(pis, pos) != 0)
        {
            std::cerr << "Error: Program compilation error.\n";
            return 1;
        }
        std::stringstream pss(pos);
        try
        {
            load_program(pss);
        }
        catch(char const* e)
        {
            std::cerr << e << '\n';
            return 1;
        }
        catch(const std::string& e)
        {
            std::cerr << e << '\n';
            return 1;
        }
    }
    std::string query;
    while (std::cin.good())
    {
        std::cout << "?- ";
        if(!std::cin.good()){
            return 0;
        }
        std::cin >> std::ws;
        getline(std::cin, query);
        std::string qos;
        if (compile("?-"+query, qos) != 0)
        {
            std::cerr << "Error: Program compilation error.\n";
            return 1;
        }
        std::stringstream qss(qos);
        try
        {
            load_query(qss);
            if (stats)
            {
                last_time = high_resolution_clock::now();
            }
            run();
            if (stats)
            {
                timer += high_resolution_clock::now() - last_time;
                std::cout << inferences << " inferences, " << timer.count() << " seconds\n";
            }
        }
        catch(char const* e)
        {
            std::cerr << e << '\n';
        }
        catch(const std::string& e)
        {
            std::cerr << e << '\n';
        }
        reset_memory();
        if (stats)
        {
            reset_stats();
        }
    }
    return 0;
}

int main(int argc, char** argv){
    if (argc > 1 && strcmp(argv[1], "-c") == 0)
    {
        if (argc > 2)
        {
            if (argc > 3)
            {
                return only_compile(argv[2], argv[3]);
            } else
            {
                return only_compile(argv[2], "a.out");
            }
        } else
        {
            std::cerr << "Error: Missing arguments.\n";
            return 1;
        }
    } else if (argc > 1 && strcmp(argv[1], "-e") == 0)
    {
        if ((argc > 3 && strcmp(argv[2], "--stats") != 0) || argc > 4)
        {
            if (strcmp(argv[2], "--stats") == 0)
            {
                stats = true;
                return only_run(argv[3], argv[4]);
            } else
            {
                return only_run(argv[2], argv[3]);
            }
        } else
        {
            std::cerr << "Error: Missing arguments.\n";
        }
    } else
    {
        std::string program_filename = "";
        if (argc > 1)
        {
            if (strcmp(argv[1], "--stats") == 0)
            {
                stats = true;
                if (argc > 2)
                {
                    program_filename = argv[2];
                }
            } else
            {
                program_filename = argv[1];
            }
        }
        return compile_and_run(program_filename);
    }
}
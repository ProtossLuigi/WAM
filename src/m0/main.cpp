#include <iostream>
#include <vector>
#include <memory>
#include "DataCell.hpp"
#include "FunctorCell.hpp"
#include <regex>
#include <fstream>

std::vector<std::shared_ptr<DataCell>> heap();
std::vector<std::shared_ptr<DataCell>> registers();

std::shared_ptr<FunctorCell> string_to_functor(std::string str){
    std::smatch match;
    std::regex_match(str, match, std::regex("(\\w+)\\/(\\d+)"));
    return std::make_shared<FunctorCell>(match[1], std::stoi(match[2]));
}

int get_structure(std::string functor, int reg){
    //TODO
    return 0;
}

void unify_variable(int reg){
    //TODO
}

void unify_value(int reg){
    //TODO
}

void put_structure(std::string functor, int reg){
    //TODO
}

void set_variable(int reg){
    //TODO
}

void set_value(int reg){
    //TODO
}

int read_program(std::ifstream f){
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
            return 2;
        }
    }
    return 0;
}

int read_query(std::ifstream f){
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
            return 2;
        }
    }
    return 0;
}

int main(int argc, char** argv){
    std::cout << "Hello, world!\n";
    return 0;
}
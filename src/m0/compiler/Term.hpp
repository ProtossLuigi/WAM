#pragma once
#include <string>
#include <vector>
#include <memory>

class Term{
public:
    std::string name;
    int no_subterms;
    std::vector<std::shared_ptr<Term>> subterms;
    Term(std::string name);
    Term(std::string name, std::vector<std::shared_ptr<Term>> subterms);
    bool operator==(const Term& t);
    bool isVar();
    std::string to_string();
};

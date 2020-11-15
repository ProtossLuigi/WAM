#include "Term.hpp"

Term::Term(std::string name){
    this->name = name;
    this->no_subterms = 0;
}

Term::Term(std::string name, std::vector<std::shared_ptr<Term>> subterms){
    this->name = name;
    this->no_subterms = subterms.size();
    this->subterms = subterms;
}

bool Term::operator==(const Term& t){
    if(name == t.name && no_subterms == t.no_subterms){
        for(unsigned int i=0; i<no_subterms; i++){
            if(!(*subterms[i] == *t.subterms[i])){
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Term::isVar(){
    return 'A' <= name[0] && 'Z' >= name[0];
}

std::string Term::to_string(){
    std::string str = name;
    if(no_subterms > 0){
        str += "(";
        for(unsigned int i=0; i<no_subterms; i++){
            if(i > 0){
                str += ",";
            }
            str += subterms[i]->to_string();
        }
        str += ")";
    }
    return str;
}
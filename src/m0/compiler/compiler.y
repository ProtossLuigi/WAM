%code top{}
%code requires{
    #include <string>
    #include <string.h>
    #include <vector>
    #include "Term.hpp"

    struct YYSTYPE{
        std::string                         str;
        unsigned int                        lineno;
        std::vector<std::shared_ptr<Term>>  ts;
        std::vector<std::shared_ptr<Term>>  regs;
    };
}
%code{
    #include <iostream>
    
    int yylex();
    int yyerror(const char*);

    
    bool query = false;

    template<typename T>
    int find_in_vector(const std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& val){
        for(int i=0; i<v.size(); i++){
            if(*v[i] == *val){
                return i;
            }
        }
        return -1;
    }

    std::vector<std::shared_ptr<Term>> merge_registers(std::vector<std::shared_ptr<Term>> regs1, const std::vector<std::shared_ptr<Term>>& regs2){
        for(std::shared_ptr<Term> reg : regs2){
            if(find_in_vector(regs1, reg) == -1){
                regs1.push_back(reg);
            }
        }
        return regs1;
    }

    std::string generate_instructions(const std::vector<std::shared_ptr<Term>>& regs, std::set<int>& seen_regs, std::shared_ptr<Term> t){
        int reg = find_in_vector(regs, t);
        if(query){
            if(seen_regs.find(reg) == seen_regs.end()){
                return "set_value X" + std::to_string(reg) + "\n";
            }
            if(t->isVar()){
                seen_regs.insert(reg);
                return "set_variable X" + std::to_string(reg) + "\n";
            }
            std::string instrs = "";
            for(int i=0; i<t->no_subterms; i++){
                if(!t->subterms[i].isVar() && seen_regs.find(find_in_vector(regs, t->subterms[i]) == seen_regs.end())){
                    instrs += generate_instructions(regs, seen_regs, t->subterms[i]);
                }
            }
            seen_regs.insert(reg);
            instrs += "put_structure " + t->name + "/" + std::to_string(t->no_subterms) + ",X" + std::to_string(reg) + "\n";
            for(int i=0; i<t->no_subterms; i++){
                instrs += generate_instructions(regs, seen_regs, t->subterms[i]);
            }
        } else{
            if(t->isVar()){
                if(seen_regs.find(reg) == seen_regs.end()){
                    return "unify_value X" + std::to_string(reg) + "\n";
                } else{
                    seen_regs.insert(reg);
                    return "unify_variable X" + std::to_string(reg) + "\n";
                }
            }
            std::string instrs = "";
            
        }
        return instrs;
    }
}
%define api.value.type {struct YYSTYPE}
%define parse.error verbose
%locations
%token VAR
%token STRUCT
%token COMMA
%token LPAR
%token RPAR
%token ERROR
%%

program:      term                      {
                                            $$.regs = merge_registers($1.ts, $1.regs);
                                        }
            | error                     {
                                            std::string msg ("Error in line " + std::to_string($1.lineno) + ".\n");
                                            yyerror(msg.c_str());
                                        }
;
term:         STRUCT LPAR terms RPAR    {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str, $3.ts));
                                            $$.regs = merge_registers($$.regs, $3.ts);
                                            $$.regs = merge_registers($$.regs, $3.regs);
                                        }
            | STRUCT                    {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str));
                                        }
            | VAR                       {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str));
                                        }
;
terms:        terms COMMA term          {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $1.ts;
                                            $$.ts.emplace_back($3.ts[0]);
                                            $$.regs = merge_registers($1.regs, $3.regs);
                                        }
            | term                      {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $1.ts;
                                            $$.regs = $1.regs;
                                        }
;
%%

int yyerror(const char* s){
    return 1;
}

int main(int argc, char** argv){
    for(int i=1; i<argc; i++){
        if(strcmp(argv[i], "-q") == 0){
            query = true;
        } else if(strcmp(argv[i], "-p") == 0){
            query = false;
        }
    }
    return 0;
}

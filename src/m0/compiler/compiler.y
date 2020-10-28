%code top{}
%code requires{
    #include <string>
    #include <string.h>
    #include <vector>
    #include "src/m0/compiler/Term.hpp"

    struct YYSTYPE{
        std::string                         str;
        unsigned int                        lineno;
        std::vector<std::shared_ptr<Term>>  ts;
        std::vector<std::shared_ptr<Term>>  regs;
    };
}
%code{
    #include <iostream>
    #include <set>
    
    int yylex();
    int yyerror(const char*);
    extern FILE *yyin;
    extern FILE *yyout;
    
    bool query = true;

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

    std::string generate_query_instructions(const std::vector<std::shared_ptr<Term>>& regs, std::set<int>& seen_regs, std::shared_ptr<Term> t){
        int reg = find_in_vector(regs, t);
        if(seen_regs.find(reg) != seen_regs.end()){
            return "set_value X" + std::to_string(reg) + "\n";
        }
        if(t->isVar()){
            seen_regs.insert(reg);
            return "set_variable X" + std::to_string(reg) + "\n";
        }
        std::string instrs = "";
        for(int i=0; i<t->no_subterms; i++){
            if(!t->subterms[i]->isVar() && seen_regs.find(find_in_vector(regs, t->subterms[i])) == seen_regs.end()){
                instrs += generate_query_instructions(regs, seen_regs, t->subterms[i]);
            }
        }
        seen_regs.insert(reg);
        instrs += "put_structure " + t->name + "/" + std::to_string(t->no_subterms) + ",X" + std::to_string(reg) + "\n";
        for(int i=0; i<t->no_subterms; i++){
            instrs += generate_query_instructions(regs, seen_regs, t->subterms[i]);
        }
        return instrs;
    }

    std::string generate_program_instructions(const std::vector<std::shared_ptr<Term>>& regs, std::set<int>& seen_regs, std::set<int>& seen_structs, std::shared_ptr<Term> t){
        int reg = find_in_vector(regs, t);
        if(t->isVar()){
            if(seen_regs.find(reg) != seen_regs.end()){
                return "unify_value X" + std::to_string(reg) + "\n";
            } else{
                seen_regs.insert(reg);
                return "unify_variable X" + std::to_string(reg) + "\n";
            }
        } else{
            std::string instrs = "";
            if(seen_structs.find(reg) == seen_structs.end()){
                seen_structs.insert(reg);
                instrs += "get_structure " + t->name + "/" + std::to_string(t->no_subterms) + ",X" + std::to_string(reg) + "\n";
                for(int i=0; i<t->no_subterms; i++){
                    int sub_reg = find_in_vector(regs, t->subterms[i]);
                    if(seen_regs.find(sub_reg) == seen_regs.end()){
                        seen_regs.insert(sub_reg);
                        instrs += "unify_variable X" + std::to_string(sub_reg) + "\n";
                    } else{
                        instrs += "unify_value X" + std::to_string(sub_reg) + "\n";
                    }
                }
                for(int i=0; i<t->no_subterms; i++){
                    int sub_reg = find_in_vector(regs, t->subterms[i]);
                    if(!t->subterms[i]->isVar() && seen_structs.find(sub_reg) == seen_structs.end()){
                        instrs += generate_program_instructions(regs, seen_regs, seen_structs, t->subterms[i]);
                    }
                }
            }
            return instrs;
        }
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
                                            for(int i=0; i<$$.regs.size(); i++){
                                                std::cout << "X" << i << " = " << $$.regs[i]->to_string() << "\n";
                                            }
                                            std::string instrs;
                                            if(query){
                                                std::set<int> s1;
                                                instrs = generate_query_instructions($$.regs, s1, $1.ts[0]);
                                            } else{
                                                std::set<int> s1, s2;
                                                instrs = generate_program_instructions($$.regs, s1, s2, $1.ts[0]);
                                            }
                                            fprintf(yyout, "%s", instrs.c_str());
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
    std::cout << s;
    return 1;
}

int main(int argc, char** argv){
    yyin = stdin;
    yyout = stdout;
    bool input_set = false;
    bool output_set = false;
    for(int i=1; i<argc; i++){
        if(strcmp(argv[i], "-q") == 0){
            query = true;
        } else if(strcmp(argv[i], "-p") == 0){
            query = false;
        } else if(strcmp(argv[i], "-i") == 0 && !input_set && argc > i+1){
            yyin = fopen(argv[i+1], "r");
            input_set = true;
            i++;
        } else if(strcmp(argv[i], "-o") == 0 && !output_set && argc > i+1){
            yyout = fopen(argv[i+1], "w");
            output_set = true;
            i++;
        }
    }
    yyparse();
    fclose(yyin);
    fclose(yyout);
    return 0;
}

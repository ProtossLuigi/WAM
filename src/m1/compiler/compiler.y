%code top{}
%code requires{
    #include <string>
    #include <string.h>
    #include <vector>
    #include "src/m1/compiler/Term.hpp"

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
    #include <queue>
    
    int yylex();
    int yyerror(const char*);
    extern FILE *yyin;
    extern FILE *yyout;
    
    bool query = true;

    template<typename T>
    int find_in_vector(const std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& val){
        for(unsigned int i=0; i<v.size(); i++){
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
        if(t->isVar() || seen_regs.find(reg) != seen_regs.end()){
            return "";
        }
        std::string instrs;
        for(int i=0; i<t->no_subterms; i++){
            instrs += generate_query_instructions(regs, seen_regs, t->subterms[i]);
        }
        seen_regs.insert(reg);
        instrs += "put_structure " + t->name + "/" + std::to_string(t->no_subterms) + ",X" + std::to_string(reg) + "\n";
        for(int i=0; i<t->no_subterms; i++){
            int reg2 = find_in_vector(regs, t->subterms[i]);
            if(seen_regs.find(reg2) == seen_regs.end()){
                seen_regs.insert(reg2);
                instrs += "set_variable X" + std::to_string(reg2) + "\n";
            } else{
                instrs += "set_value X" + std::to_string(reg2) + "\n";
            }
        }
        return instrs;
    }

    std::string compile_query(const std::vector<std::shared_ptr<Term>>& regs){
        std::shared_ptr<Term> t = regs[0];
        std::set<int> seen_regs;
        std::string instrs;
        for(int i=0; i<t->no_subterms; i++){
            int reg = find_in_vector(regs, t->subterms[i]);
            if(seen_regs.find(reg) != seen_regs.end()){
                instrs += "put_value X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
            } else if(t->subterms[i]->isVar()){
                seen_regs.insert(reg);
                instrs += "put_variable X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
            } else{
                for(int j=0; j<t->subterms[i]->no_subterms; j++){
                    instrs += generate_query_instructions(regs, seen_regs, t->subterms[i]->subterms[j]);
                }
                instrs += "put_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                for(int j=0; j<t->subterms[i]->no_subterms; j++){
                    int reg2 = find_in_vector(regs, t->subterms[i]->subterms[j]);
                    if(seen_regs.find(reg2) == seen_regs.end()){
                        seen_regs.insert(reg2);
                        instrs += "set_variable X" + std::to_string(reg2) + "\n";
                    } else{
                        instrs += "set_value X" + std::to_string(reg2) + "\n";
                    }
                }
            }
        }
        instrs += "call " + t->name + "/" + std::to_string(t->no_subterms) + "\n";
        return instrs;
    }

    std::string generate_program_instructions(const std::vector<std::shared_ptr<Term>>& regs, std::set<int>& seen_regs, std::shared_ptr<Term> t, std::queue<int>& q){
        int reg = find_in_vector(regs, t);
        std::string instrs;
        if(seen_regs.find(reg) == seen_regs.end()){
            seen_regs.insert(reg);
            instrs += "unify_variable X" + std::to_string(reg) + "\n";
        } else{
            instrs += "unify_value X" + std::to_string(reg) + "\n";
        }
        if(!t->isVar()){
            q.push(reg);
        }
        return instrs;
    }

    std::string compile_program(const std::vector<std::shared_ptr<Term>>& regs){
        std::shared_ptr<Term> t = regs[0];
        std::set<int> seen_regs;
        std::queue<int> q;
        std::string instrs = t->name + "/" + std::to_string(t->no_subterms) + " :\n";
        for(int i=0; i<t->no_subterms; i++){
            int reg = find_in_vector(regs, t->subterms[i]);
            if(seen_regs.find(reg) != seen_regs.end()){
                instrs += "get_value X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
            } else if(t->subterms[i]->isVar()){
                seen_regs.insert(reg);
                instrs += "get_variable X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
            } else{
                instrs += "get_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                for(int j=0; j<t->subterms[i]->no_subterms; j++){
                    instrs += generate_program_instructions(regs, seen_regs, t->subterms[i]->subterms[j], q);
                }
            }
        }
        std::set<int> constructed;
        while(!q.empty()){
            if(constructed.find(q.front()) == constructed.end()){
                instrs += "get_structure " + regs[q.front()]->name + "/" + std::to_string(regs[q.front()]->no_subterms) + ",X" + std::to_string(q.front()) + "\n";
                for(int i=0; i<regs[q.front()]->no_subterms; i++){
                    instrs += generate_program_instructions(regs, seen_regs, regs[q.front()]->subterms[i], q);
                }
            }
            q.pop();
        }
        instrs += "proceed\n";
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
%token DOT
%token ERROR
%%

program:      predicates                {
                                            fprintf(yyout, "%s", $1.str.c_str());
                                        }
            | error                     {
                                            std::string msg ("Error in line " + std::to_string($1.lineno) + ".\n");
                                            yyerror(msg.c_str());
                                        }
;
predicates:   predicates predicate      {
                                            if(query){
                                                yyerror("invalid query syntax\n");
                                                return 1;
                                            }
                                            $$.lineno = $1.lineno;
                                            $$.str = $1.str + $2.str;
                                        }
            | predicate                 {
                                            $$.lineno = $1.lineno;
                                            $$.str = $1.str;
                                        }
;
predicate:    term DOT                  {
                                            $$.lineno = $1.lineno;
                                            $$.regs = merge_registers($1.ts, $1.regs);
                                            std::vector<std::shared_ptr<Term>> arg_regs = $1.ts[0]->subterms;
                                            for(unsigned int i=0; i<$$.regs.size(); i++){
                                                std::cout << "X" << i << " = " << $$.regs[i]->to_string() << "\n";
                                            }
                                            for(unsigned int i=0; i<arg_regs.size(); i++){
                                                std::cout << "A" << i << " = " << arg_regs[i]->to_string() << "\n";
                                            }
                                            if(query){
                                                $$.str = compile_query($$.regs);
                                            } else{
                                                $$.str = compile_program($$.regs);
                                            }
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

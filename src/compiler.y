%code top{}
%code requires{
    #include <string>
    #include <string.h>
    #include <vector>
    #include "src/Term.hpp"

    struct YYSTYPE{
        std::string                         str;
        unsigned int                        lineno;
        std::vector<std::shared_ptr<Term>>  ts;
        std::vector<std::string>            codes;
    };
}
%code{
    #include <iostream>
    #include <set>
    #include <queue>
    #include <unordered_map>
    #include <sstream>
    
    int yylex();
    int yyerror(const char*);
    extern FILE *yyin;
    extern FILE *yyout;
    extern int main_loop(int rgc, char** argv);

    std::string ostring;

    template<typename T>
    int find_in_vector(const std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& val){
        for(unsigned int i=0; i<v.size(); i++){
            if(*v[i] == *val){
                return i;
            }
        }
        return -1;
    }

    template<typename T>
    void remove_from_vector(std::vector<T>& v, int n){
        for(unsigned int i=n; i<v.size()-1; i++){
            v[i] = v[i+1];
        }
        v.pop_back();
    }

    std::vector<std::shared_ptr<Term>> allocate_registers(const std::shared_ptr<Term>& t, std::vector<std::shared_ptr<Term>>& vars){
        std::vector<std::shared_ptr<Term>> regs;
        std::queue<std::shared_ptr<Term>> q;
        for(unsigned int i=0; i<t->no_subterms; i++){
            if(t->subterms[i]->isVar()){
                if(find_in_vector(regs, t->subterms[i]) == -1){
                    regs.emplace_back(t->subterms[i]);
                    vars.emplace_back(t->subterms[i]);
                }
            } else{
                for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                    q.push(t->subterms[i]->subterms[j]);
                }
            }
        }
        while(!q.empty()){
            if(find_in_vector(regs, q.front()) == -1){
                regs.emplace_back(q.front());
                if(q.front()->isVar()){
                    vars.emplace_back(q.front());
                } else{
                    for(unsigned int i=0; i<q.front()->no_subterms; i++){
                        q.push(q.front()->subterms[i]);
                    }
                }
            }
            q.pop();
        }
        return regs;
    }

    std::string generate_query_instructions(const std::vector<std::shared_ptr<Term>>& temp_regs, std::set<int>& seen_temp_regs, const std::vector<std::shared_ptr<Term>>& perm_regs, std::set<int>& seen_perm_regs, std::shared_ptr<Term> t){
        int reg = find_in_vector(temp_regs, t);
        if(t->isVar() || seen_temp_regs.find(reg) != seen_temp_regs.end()){
            return "";
        }
        std::string instrs;
        for(unsigned int i=0; i<t->no_subterms; i++){
            instrs += generate_query_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, t->subterms[i]);
        }
        seen_temp_regs.insert(reg);
        instrs += "put_structure " + t->name + "/" + std::to_string(t->no_subterms) + ",X" + std::to_string(reg) + "\n";
        for(unsigned int i=0; i<t->no_subterms; i++){
            int reg2;
            if((reg2 = find_in_vector(temp_regs, t->subterms[i])) == -1){
                reg2 = find_in_vector(perm_regs, t->subterms[i]);
                if(seen_perm_regs.find(reg2) == seen_perm_regs.end()){
                    seen_perm_regs.insert(reg2);
                    instrs += "set_variable Y" + std::to_string(reg2) + "\n";
                } else{
                    instrs += "set_value Y" + std::to_string(reg2) + "\n";
                }
            } else{
                if(seen_temp_regs.find(reg2) == seen_temp_regs.end()){
                    seen_temp_regs.insert(reg2);
                    instrs += "set_variable X" + std::to_string(reg2) + "\n";
                } else{
                    instrs += "set_value X" + std::to_string(reg2) + "\n";
                }
            }
        }
        return instrs;
    }

    std::string compile_query(const std::vector<std::shared_ptr<Term>>& temp_regs, const std::vector<std::shared_ptr<Term>>& perm_regs, std::set<int>& seen_perm_regs, std::shared_ptr<Term> t){
        std::set<int> seen_temp_regs;
        std::string instrs;
        for(unsigned int i=0; i<t->no_subterms; i++){
            int reg;
            if((reg = find_in_vector(perm_regs, t->subterms[i])) != -1){
                if(seen_perm_regs.find(reg) != seen_perm_regs.end()){
                    instrs += "put_value Y" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else{
                    seen_perm_regs.insert(reg);
                    instrs += "put_variable Y" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                }
            } else if((reg = find_in_vector(temp_regs, t->subterms[i])) != -1){
                if(seen_temp_regs.find(reg) != seen_temp_regs.end()){
                    instrs += "put_value X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else if(t->subterms[i]->isVar()){
                    seen_temp_regs.insert(reg);
                    instrs += "put_variable X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else{
                    for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                        instrs += generate_query_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, t->subterms[i]->subterms[j]);
                    }
                    instrs += "put_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                    for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                        int reg2;
                        if((reg2 = find_in_vector(temp_regs, t->subterms[i]->subterms[j])) == -1){
                            reg2 = find_in_vector(perm_regs, t->subterms[i]->subterms[j]);
                            if(seen_perm_regs.find(reg2) == seen_perm_regs.end()){
                                seen_perm_regs.insert(reg2);
                                instrs += "set_variable Y" + std::to_string(reg2) + "\n";
                            } else{
                                instrs += "set_value Y" + std::to_string(reg2) + "\n";
                            }
                        } else{
                            if(seen_temp_regs.find(reg2) == seen_temp_regs.end()){
                                seen_temp_regs.insert(reg2);
                                instrs += "set_variable X" + std::to_string(reg2) + "\n";
                            } else{
                                instrs += "set_value X" + std::to_string(reg2) + "\n";
                            }
                        }
                    }
                }
            } else{
                for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                    instrs += generate_query_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, t->subterms[i]->subterms[j]);
                }
                instrs += "put_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                    int reg2;
                    if((reg2 = find_in_vector(temp_regs, t->subterms[i]->subterms[j])) == -1){
                        reg2 = find_in_vector(perm_regs, t->subterms[i]->subterms[j]);
                        if(seen_perm_regs.find(reg2) == seen_perm_regs.end()){
                            seen_perm_regs.insert(reg2);
                            instrs += "set_variable Y" + std::to_string(reg2) + "\n";
                        } else{
                            instrs += "set_value Y" + std::to_string(reg2) + "\n";
                        }
                    } else{
                        if(seen_temp_regs.find(reg2) == seen_temp_regs.end()){
                            seen_temp_regs.insert(reg2);
                            instrs += "set_variable X" + std::to_string(reg2) + "\n";
                        } else{
                            instrs += "set_value X" + std::to_string(reg2) + "\n";
                        }
                    }
                }
            }
            
        }
        instrs += "call " + t->name + "/" + std::to_string(t->no_subterms) + "\n";
        return instrs;
    }

    std::string generate_program_instructions(const std::vector<std::shared_ptr<Term>>& temp_regs, std::set<int>& seen_temp_regs, const std::vector<std::shared_ptr<Term>>& perm_regs, std::set<int>& seen_perm_regs, std::shared_ptr<Term> t, std::queue<int>& q){
        std::string instrs;
        int reg;
        if((reg = find_in_vector(temp_regs, t)) == -1){
            reg = find_in_vector(perm_regs, t);
            if(seen_perm_regs.find(reg) == seen_perm_regs.end()){
                seen_perm_regs.insert(reg);
                instrs += "unify_variable Y" + std::to_string(reg) + "\n";
            } else{
                instrs += "unify_value Y" + std::to_string(reg) + "\n";
            }
        } else{
            if(seen_temp_regs.find(reg) == seen_temp_regs.end()){
                seen_temp_regs.insert(reg);
                instrs += "unify_variable X" + std::to_string(reg) + "\n";
            } else{
                instrs += "unify_value X" + std::to_string(reg) + "\n";
            }
            if(!t->isVar()){
                q.push(reg);
            }
        }
        return instrs;
    }

    std::string compile_head(const std::vector<std::shared_ptr<Term>>& temp_regs, const std::vector<std::shared_ptr<Term>>& perm_regs, std::set<int>& seen_perm_regs, std::shared_ptr<Term> t){
        std::set<int> seen_temp_regs;
        std::queue<int> q;
        std::string instrs;
        for(unsigned int i=0; i<t->no_subterms; i++){
            int reg;
            if((reg = find_in_vector(perm_regs, t->subterms[i])) != -1){
                if(seen_perm_regs.find(reg) == seen_perm_regs.end()){
                    seen_perm_regs.insert(reg);
                    instrs += "get_variable Y" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else{
                    instrs += "get_value Y" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                }
            } else if((reg = find_in_vector(temp_regs, t->subterms[i])) != -1){
                if(seen_temp_regs.find(reg) != seen_temp_regs.end()){
                    instrs += "get_value X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else if(t->subterms[i]->isVar()){
                    seen_temp_regs.insert(reg);
                    instrs += "get_variable X" + std::to_string(reg) + ",A" + std::to_string(i) + "\n";
                } else{
                    instrs += "get_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                    for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                        instrs += generate_program_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, t->subterms[i]->subterms[j], q);
                    }
                }
            } else{
                instrs += "get_structure " + t->subterms[i]->name + "/" + std::to_string(t->subterms[i]->no_subterms) + ",A" + std::to_string(i) + "\n";
                for(unsigned int j=0; j<t->subterms[i]->no_subterms; j++){
                    instrs += generate_program_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, t->subterms[i]->subterms[j], q);
                }
            }
        }
        std::set<int> constructed;
        while(!q.empty()){
            if(constructed.find(q.front()) == constructed.end()){
                instrs += "get_structure " + temp_regs[q.front()]->name + "/" + std::to_string(temp_regs[q.front()]->no_subterms) + ",X" + std::to_string(q.front()) + "\n";
                for(unsigned int i=0; i<temp_regs[q.front()]->no_subterms; i++){
                    instrs += generate_program_instructions(temp_regs, seen_temp_regs, perm_regs, seen_perm_regs, temp_regs[q.front()]->subterms[i], q);
                }
            }
            q.pop();
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
%token LBR
%token RBR
%token DOT
%token IMPLICATION
%token QUERY
%token EQUALS
%token VERT
%token ERROR
%%

program:      predicates                {
                                            std::unordered_map<std::string, std::vector<unsigned int>> clauses;
                                            for(unsigned int i=0; i<$1.ts.size(); i++){
                                                std::string name = $1.ts[i]->name + "/" + std::to_string($1.ts[i]->no_subterms);
                                                if(clauses.find(name) == clauses.end()){
                                                    clauses[name] = std::vector<unsigned int>();
                                                }
                                                clauses[name].push_back(i);
                                            }
                                            int labels = 0;
                                            std::string code;
                                            for(std::pair<std::string, std::vector<unsigned int>> clause : clauses){
                                                code += clause.first + " :\n";
                                                if(clause.second.size() == 1){
                                                    code += $1.codes[clause.second[0]];
                                                } else{
                                                    for(unsigned int i=0; i<clause.second.size(); i++){
                                                        if(i == 0){
                                                            code += "try_me_else L" + std::to_string(labels) + "\n";
                                                        } else{
                                                            code += "L" + std::to_string(labels) + " :\n";
                                                            labels++;
                                                            if(i == clause.second.size()-1){
                                                                code += "trust_me\n";
                                                            } else{
                                                                code += "retry_me_else L" + std::to_string(labels) + "\n";
                                                            }
                                                        }
                                                        code += $1.codes[clause.second[i]];
                                                    }
                                                }
                                            }
                                            ostring = code;
                                            YYACCEPT;
                                        }
            | QUERY terms DOT           {
                                            std::vector<std::vector<std::shared_ptr<Term>>> regs;
                                            std::vector<std::shared_ptr<Term>> temp_vars;
                                            std::vector<std::shared_ptr<Term>> perm_vars;
                                            for(unsigned int i=0; i<$2.ts.size(); i++){
                                                std::vector<std::shared_ptr<Term>> local_vars;
                                                regs.push_back(allocate_registers($2.ts[i], local_vars));
                                                for(std::shared_ptr<Term> t : local_vars){
                                                    if(find_in_vector(perm_vars, t) == -1){
                                                        int r;
                                                        if((r = find_in_vector(temp_vars, t)) == -1){
                                                            temp_vars.emplace_back(t);
                                                        } else{
                                                            remove_from_vector(temp_vars, r);
                                                            perm_vars.emplace_back(t);
                                                        }
                                                    }
                                                }
                                            }
                                            for(unsigned int i=0; i<regs.size(); i++){
                                                for(std::shared_ptr<Term> t : perm_vars){
                                                    int r;
                                                    if((r = find_in_vector(regs[i], t)) != -1){
                                                        remove_from_vector(regs[i], r);
                                                    }
                                                }
                                            }
                                            std::string code = "allocate " + std::to_string(perm_vars.size()) + "\n";
                                            std::set<int> seen_perm_regs;
                                            for(unsigned int i=0; i<$2.ts.size(); i++){
                                                code += compile_query(regs[i], perm_vars, seen_perm_regs, $2.ts[i]);
                                            }
                                            ostring = code;
                                            YYACCEPT;
                                        }
            | error                     {
                                            std::string msg ("Error in line " + std::to_string($1.lineno) + ".\n");
                                            yyerror(msg.c_str());
                                            YYABORT;
                                        }
;
predicates:   predicates predicate      {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $1.ts;
                                            $$.ts.emplace_back($2.ts[0]);
                                            $$.codes = $1.codes;
                                            $$.codes.push_back($2.str);
                                        }
            | predicate                 {
                                            $$.lineno = $1.lineno;
                                            $$.ts = std::vector<std::shared_ptr<Term>>();
                                            $$.ts.emplace_back($1.ts[0]);
                                            $$.codes.push_back($1.str);
                                        }
;
predicate:        term IMPLICATION terms DOT    {
                                                    $$.ts = $1.ts;
                                                    $$.ts.insert($$.ts.end(), $3.ts.begin(), $3.ts.end());
                                                    std::vector<std::vector<std::shared_ptr<Term>>> regs;
                                                    std::vector<std::shared_ptr<Term>> temp_vars;
                                                    std::vector<std::shared_ptr<Term>> perm_vars;
                                                    for(unsigned int i=0; i<$$.ts.size(); i++){
                                                        std::vector<std::shared_ptr<Term>> local_vars;
                                                        regs.push_back(allocate_registers($$.ts[i], local_vars));
                                                        for(std::shared_ptr<Term> t : local_vars){
                                                            if(find_in_vector(perm_vars, t) == -1){
                                                                int r;
                                                                if((r = find_in_vector(temp_vars, t)) == -1){
                                                                    temp_vars.emplace_back(t);
                                                                } else{
                                                                    remove_from_vector(temp_vars, r);
                                                                    perm_vars.emplace_back(t);
                                                                }
                                                            }
                                                        }
                                                    }
                                                    for(unsigned int i=0; i<regs.size(); i++){
                                                        for(std::shared_ptr<Term> t : perm_vars){
                                                            int r;
                                                            if((r = find_in_vector(regs[i], t)) != -1){
                                                                remove_from_vector(regs[i], r);
                                                            }
                                                        }
                                                    }
                                                    $$.str = "allocate " + std::to_string(perm_vars.size()) + "\n";
                                                    std::set<int> seen_perm_regs;
                                                    $$.str += compile_head(regs[0], perm_vars, seen_perm_regs, $$.ts[0]);
                                                    for(unsigned int i=1; i<$$.ts.size(); i++){
                                                        $$.str += compile_query(regs[i], perm_vars, seen_perm_regs, $$.ts[i]);
                                                    }
                                                    $$.str += "deallocate\n";
                                                }
                | term DOT                      {
                                                    $$.lineno = $1.lineno;
                                                    $$.ts = $1.ts;
                                                    std::vector<std::shared_ptr<Term>> temp_vars;
                                                    std::vector<std::shared_ptr<Term>> temp_regs = allocate_registers($1.ts[0], temp_vars);
                                                    std::vector<std::shared_ptr<Term>> perm_regs;
                                                    std::set<int> seen_perm_regs;
                                                    $$.str = compile_head(temp_regs, perm_regs, seen_perm_regs, $1.ts[0]);
                                                    $$.str += "proceed\n";
                                                }
;
term:         STRUCT LPAR terms RPAR    {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str, $3.ts));
                                        }
            | STRUCT                    {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str));
                                        }
            | VAR                       {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>($1.str));
                                        }
            | term EQUALS term          {
                                            $$.lineno = $1.lineno;
                                            $$.ts = std::vector<std::shared_ptr<Term>>();
                                            $$.ts.push_back(std::make_shared<Term>("="));
                                            $$.ts[0]->no_subterms = 2;
                                            $$.ts[0]->subterms.push_back($1.ts[0]);
                                            $$.ts[0]->subterms.push_back($3.ts[0]);
                                        }
            | LBR RBR                   {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>("[]"));
                                        }
            | LBR terms RBR             {
                                            $$.lineno = $1.lineno;
                                            $$.ts.push_back(std::make_shared<Term>("[]"));
                                            for(int i=$2.ts.size()-1; i>-1; i--){
                                                std::shared_ptr<Term> list_term = std::make_shared<Term>(".");
                                                list_term->no_subterms = 2;
                                                list_term->subterms.push_back($2.ts[i]);
                                                list_term->subterms.push_back($$.ts[0]);
                                                $$.ts[0] = list_term;
                                            }
                                        }
            | LBR terms VERT term RBR   {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $4.ts;
                                            for(int i=$2.ts.size()-1; i>-1; i--){
                                                std::shared_ptr<Term> list_term = std::make_shared<Term>(".");
                                                list_term->no_subterms = 2;
                                                list_term->subterms.push_back($2.ts[i]);
                                                list_term->subterms.push_back($$.ts[0]);
                                                $$.ts[0] = list_term;
                                            }
                                        }
;
terms:        terms COMMA term          {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $1.ts;
                                            $$.ts.emplace_back($3.ts[0]);
                                        }
            | term                      {
                                            $$.lineno = $1.lineno;
                                            $$.ts = $1.ts;
                                        }
;
%%

int yyerror(const char* s){
    std::cout << s;
    return 1;
}

ssize_t my_read(void* cookie, char* buf, size_t size){
    ((std::stringstream*)cookie)->read(buf, size);
    if(((std::stringstream*)cookie)->fail()){
        return -1;
    } else{
        return strlen(buf);
    }
}

ssize_t my_write(void *cookie, const char *buf, size_t size){
    ((std::stringstream*)cookie)->write(buf, size);
    if(((std::stringstream*)cookie)->fail()){
        return 0;
    } else{
        return size;
    }
}

int my_in_seek(void *cookie, off64_t *offset, int whence){
    std::ios_base::seekdir way;
    switch(whence){
    case SEEK_SET:
        way = std::ios_base::beg;
        break;
    case SEEK_CUR:
        way = std::ios_base::cur;
        break;
    case SEEK_END:
        way = std::ios_base::end;
        break;
    default:
        return -1;
    }
    ((std::stringstream*)cookie)->seekg(*offset, way);
    *offset = ((std::stringstream*)cookie)->tellg();
    if(((std::stringstream*)cookie)->fail()){
        return -1;
    } else{
        return 0;
    }
}

int my_out_seek(void *cookie, off64_t *offset, int whence){
    std::ios_base::seekdir way;
    switch(whence){
    case SEEK_SET:
        way = std::ios_base::beg;
        break;
    case SEEK_CUR:
        way = std::ios_base::cur;
        break;
    case SEEK_END:
        way = std::ios_base::end;
        break;
    default:
        return -1;
    }
    ((std::stringstream*)cookie)->seekp(*offset, way);
    *offset = ((std::stringstream*)cookie)->tellp();
    if(((std::stringstream*)cookie)->fail()){
        return -1;
    } else{
        return 0;
    }
} 



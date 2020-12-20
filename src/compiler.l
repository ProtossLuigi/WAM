%option noyywrap
%{
    #include "compiler_y.hpp"
    #include <string>
    int yylex();
    extern std::string ostring;
%}
%option yylineno
%%
[ \t\n]+
\%.*$
[A-Z][[:alnum:]_]*      {yylval.lineno = yylineno; yylval.str = std::string(yytext); return VAR;}
[a-z0-9_][[:alnum:]_]*  {yylval.lineno = yylineno; yylval.str = std::string(yytext); return STRUCT;}
\,                      {yylval.lineno = yylineno; return COMMA;}
\(                      {yylval.lineno = yylineno; return LPAR;}
\)                      {yylval.lineno = yylineno; return RPAR;}
\.                      {yylval.lineno = yylineno; return DOT;}
\:\-                    {yylval.lineno = yylineno; return IMPLICATION;}
\?\-                    {yylval.lineno = yylineno; return QUERY;}
=                       {yylval.lineno = yylineno; return EQUALS;}
\[                      {yylval.lineno = yylineno; return LBR;}
\]                      {yylval.lineno = yylineno; return RBR;}
\|                      {yylval.lineno = yylineno; return VERT;}
.
%%

int compile(std::string in_string, std::string& out_string){
    yy_scan_string(in_string.c_str());
    int result = yyparse();
    out_string = ostring;
    yy_delete_buffer(YY_CURRENT_BUFFER);
    return result;
}
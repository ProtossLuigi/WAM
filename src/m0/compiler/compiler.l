%option noyywrap
%{
    #include "compiler_y.hpp"
    #include <string>
    int yylex();
%}
%option yylineno
%%
[ \t\n]+
[A-Z][:alnum:]*         {yylval.lineno = yylineno; yylval.str = std::string(yytext); return VAR;}
[a-z][:alnum:]*         {yylval.lineno = yylineno; yylval.str = std::string(yytext); return STRUCT;}
\,                      {yylval.lineno = yylineno; return COMMA;}
\(                      {yylval.lineno = yylineno; return LPAR;}
\)                      {yylval.lineno = yylineno; return RPAR;}
.                       {yylval.lineno = yylineno; return ERROR;}
%%
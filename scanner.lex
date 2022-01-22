%{
/* declerations section */
#include "parser_utility.hpp"
#include "hw5_output.hpp"
#include "parser.tab.hpp"





%}

%option noyywrap
%option yylineno
%x end_lex

whitespace	([\t\r\n ])


%%
void                                            return VOID;
int                                             return INT;
byte                                            return BYTE;
b                                               return B;
bool                                            return BOOL;
const                                           return CONST;
and                                             return AND;
or                                              return OR;
not                                             return NOT; 
true                                            return TRUE;
false                                           return FALSE;
return                                          return RETURN;
if                                              return IF;
else                                            return ELSE;
while                                           return WHILE;
break                                           return BREAK; 
continue                                        return CONTINUE;
;                                               return SC;
,                                               return COMMA;
\(                                              return LPAREN;
\)                                              return RPAREN;
\{                                              return LBRACE;
\}                                              return RBRACE;
=                                               return ASSIGN;
[<>]=?                                          {yylval.data = yytext; return RELOP;}
[!=]=                                           {yylval.data = yytext; return EQUALOP;} 
(\+)                                            return ADD;
(\-)                                            return SUB;
(\*)                                            return MULT;
(\/)                                            return DIV;
\/\/[^\n\r]*[\r|\n|\r\n]?                       ;
[a-zA-Z][a-zA-Z0-9]*                            {yylval.data = yytext; return ID;}
0|[1-9][0-9]*                                   {yylval.data = yytext; return NUM;}
\"([^\n\r\"\\]|\\[rnt"\\])+\"                   return STRING;
{whitespace}                                    ;
.                                               {output::errorLex(yylineno); exit(0);}
%%

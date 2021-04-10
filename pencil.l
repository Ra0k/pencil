%option noyywrap yylineno

%{
  
#include <stdio.h>
#include "semantics.h"

#define YY_DECL int yylex()
#include "pencil.tab.h"
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

DIGIT       [0-9]
WS          [ \t\n]
LETTER      [a-zA-Z]

INT         {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
VAR         {LETTER}({LETTER}|{DIGIT}|_)*

%%

program     return T_PROGRAM;
begin       return T_BEGIN;
end         return T_END;
integer     return T_INTEGER; 
boolean     return T_BOOLEAN;
skip        return T_SKIP;
if          return T_IF;
elif        return T_ELIF;
then        return T_THEN;
else        return T_ELSE;
endif       return T_ENDIF;
while       return T_WHILE;
do          return T_DO;
done        return T_DONE;
read        return T_READ;
write       return T_WRITE;
";"         return T_SEMICOLON;
":="        return T_ASSIGN;
"="         return T_EQ;
"<"         return T_LESS;
">"         return T_GR;
"+"         return T_ADD;
"-"         return T_SUB;
"*"         return T_MUL;
and         return T_AND;
or          return T_OR;
not         return T_NOT;
div         return T_DIV; 
mod         return T_MOD;
"("         return T_OPEN;
")"         return T_CLOSE;
"{"         return T_OPEN_BRACES;
"}"         return T_CLOSE_BRACES;
"~"         return T_TILDE;
","         return T_COMMA;
"<-"        return T_BACK_ARROW;

{INT} 	    {yylval.name = new std::string(yytext); return T_NUM; }//{yylval.ival = strdup(yytext)}
{FLOAT}     {yylval.name = new std::string(yytext); return T_FLOAT; } 
true        return T_TRUE;
false       return T_FALSE;

"#".*\n     ;

{VAR}       {yylval.name = new std::string(yytext); return T_ID;}

{WS}+       ;

%%

%{
#include <stdlib.h>
#include <stdio.h>
#include "syntax.tab.h"
#include "syntax_tree.h"
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
int is_error = 0;
%}
%option yylineno
delim       [ \n\t\r]
ws          {delim}+
letter_     [A-Za-z_]
digit       [0-9]

%%
{delim}+                                        { /*do nothing*/}
0[0-7]*|0[xX][0-9a-fA-F]*|[1-9][0-9]*|0         { 
    int i = strtol(yytext, NULL, 0);
    sprintf(yytext, "%d", i);
    yylval.type_node = create_node(INT, yylineno, yytext, 0, 0);
    return INT; 
}
{digit}+"."{digit}+                             { 
    float i = atof(yytext);
    char temp[32] = "\0";
    sprintf(temp, "%f", i);
    yylval.type_node = create_node(FLOAT, yylineno, temp, 0, 0);
    return FLOAT; 
}
";"                                             { 
    yylval.type_node = create_node(SEMI, yylineno, yytext, 0, 0);
    return SEMI; 
}
","                                             { 
    yylval.type_node = create_node(COMMA, yylineno, yytext, 0, 0);
    return COMMA; 
}                                                     
"="                                             { 
    yylval.type_node = create_node(ASSIGNOP, yylineno, yytext, 0, 0);
    return ASSIGNOP; 
}
">"|"<"|">="|"<="|"=="|"!="                     { 
    yylval.type_node = create_node(RELOP, yylineno, yytext, 0, 0);
    return RELOP; 
}
"+"                                             { 
    yylval.type_node = create_node(PLUS, yylineno, yytext, 0, 0);
    return PLUS; 
}
"-"                                             { 
    yylval.type_node = create_node(MINUS, yylineno, yytext, 0, 0);
    return MINUS; 
}
"*"                                             { 
    yylval.type_node = create_node(STAR, yylineno, yytext, 0, 0);
    return STAR; 
}
"/"                                             { 
    yylval.type_node = create_node(DIV, yylineno, yytext, 0, 0);
    return DIV; 
}
"&&"                                            { 
    yylval.type_node = create_node(AND, yylineno, yytext, 0, 0);
    return AND; 
}
"||"                                            { 
    yylval.type_node = create_node(OR, yylineno, yytext, 0, 0);
    return OR; 
}
"."                                             { 
    yylval.type_node = create_node(DOT, yylineno, yytext, 0, 0);
    return DOT; 
}
"!"                                             { 
    yylval.type_node = create_node(NOT, yylineno, yytext, 0, 0);
    return NOT; 
}
"int"|"float"                                   { 
    yylval.type_node = create_node(TYPE, yylineno, yytext, 0, 0);
    return TYPE; 
}
"("                                             { 
    yylval.type_node = create_node(LP, yylineno, yytext, 0, 0);
    return LP; 
}
")"                                             { 
    yylval.type_node = create_node(RP, yylineno, yytext, 0, 0);
    return RP; 
}
"["                                             { 
    yylval.type_node = create_node(LB, yylineno, yytext, 0, 0);
    return LB; 
}
"]"                                             { 
    yylval.type_node = create_node(RB, yylineno, yytext, 0, 0);
    return RB; 
}
"{"                                             { 
    yylval.type_node = create_node(LC, yylineno, yytext, 0, 0);
    return LC; 
}
"}"                                             { 
    yylval.type_node = create_node(RC, yylineno, yytext, 0, 0);
    return RC; 
}
"struct"                                        { 
    yylval.type_node = create_node(STRUCT, yylineno, yytext, 0, 0);
    return STRUCT; 
}
"return"                                        { 
    yylval.type_node = create_node(RETURN, yylineno, yytext, 0, 0);
    return RETURN; 
}
"if"                                            { 
    yylval.type_node = create_node(IF, yylineno, yytext, 0, 0);
    return IF; 
}
"else"                                          { 
    yylval.type_node = create_node(ELSE, yylineno, yytext, 0, 0);
    return ELSE; 
}
"while"                                         { 
    yylval.type_node = create_node(WHILE, yylineno, yytext, 0, 0);
    return WHILE; 
}
{letter_}({letter_}|{digit})*                   { 
    yylval.type_node = create_node(ID, yylineno, yytext, 0, 0);
    return ID; 
}
.                                               { 
    printf("Error type A at Line %d: lexical error.\n", yylineno); 
    is_error = 1;
}
%%
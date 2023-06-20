%{
#include <stdio.h>
#include "semantic.h"
#include "hashmap.h"
#include "assemble.h"

node *syntax_root = NULL;
%}

/* declared types */
%union {
node *type_node;
}

/* declared tokens */
%token <type_node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE 

/* declared non-terminals */
%type <type_node> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList Stmt DefList Def DecList Dec Exp Args 

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB LC RC DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%% 

Program : ExtDefList { 
    $$ = create_node(Program, @$.first_line, "", 0, 1, $1);
    syntax_root = $$;
}
;
ExtDefList : /* empty */ {
    $$ = create_node(Epsilon, Epsilon, "", 0, 0);
}
| ExtDef ExtDefList {
    $$ = create_node(ExtDefList, @$.first_line, "", 1, 2, $1, $2);
}
;
ExtDef : Specifier ExtDecList SEMI {
    $$ = create_node(ExtDef, @$.first_line, "", 0, 3, $1, $2, $3);
}
| Specifier SEMI {
    $$ = create_node(ExtDef, @$.first_line, "", 1, 2, $1, $2);
}
| Specifier FunDec CompSt {
    $$ = create_node(ExtDef, @$.first_line, "", 2, 3, $1, $2, $3);
}
| error SEMI
;
ExtDecList : VarDec {
    $$ = create_node(ExtDecList, @$.first_line, "", 0, 1, $1);
}
| VarDec COMMA ExtDecList {
    $$ = create_node(ExtDecList, @$.first_line, "", 1, 3, $1, $2, $3);
}
;
Specifier : TYPE {
    $$ = create_node(Specifier, @$.first_line, "", 0, 1, $1);
}
| StructSpecifier {
    $$ = create_node(Specifier, @$.first_line, "", 1, 1, $1);
}
;
StructSpecifier : STRUCT OptTag LC DefList RC {
    $$ = create_node(StructSpecifier, @$.first_line, "", 0, 5, $1, $2, $3, $4, $5);
}
| STRUCT Tag {
    $$ = create_node(StructSpecifier, @$.first_line, "", 1, 2, $1, $2);
}
;
OptTag : /* empty */ {
    $$ = create_node(Epsilon, Epsilon, "", 0, 0);
}
| ID {
    $$ = create_node(OptTag, @$.first_line, "", 1, 1, $1);
}
;
Tag : ID {
    $$ = create_node(Tag, @$.first_line, "", 0, 1, $1);
}
;
VarDec : ID {
    $$ = create_node(VarDec, @$.first_line, "", 0, 1, $1);
}
| VarDec LB INT RB {
    $$ = create_node(VarDec, @$.first_line, "", 1, 4, $1, $2, $3, $4);
}
| VarDec LB error RB
;
FunDec : ID LP VarList RP {
    $$ = create_node(FunDec, @$.first_line, "", 0, 4, $1, $2, $3, $4);
}
| ID LP RP {
    $$ = create_node(FunDec, @$.first_line, "", 1, 3, $1, $2, $3);
}
| error RP
;
VarList : ParamDec COMMA VarList {
    $$ = create_node(VarList, @$.first_line, "", 0, 3, $1, $2, $3);
}
| ParamDec {
    $$ = create_node(VarList, @$.first_line, "", 1, 1, $1);
}
;
ParamDec : Specifier VarDec {
    $$ = create_node(ParamDec, @$.first_line, "", 0, 2, $1, $2);
}
| error COMMA
| error RP
;
CompSt : LC DefList StmtList RC {
    $$ = create_node(CompSt, @$.first_line, "", 0, 4, $1, $2, $3, $4);
}
| LC error RC
;
StmtList : /* empty */ {
    $$ = create_node(Epsilon, Epsilon, "", 0, 0);
}
| Stmt StmtList {
    $$ = create_node(StmtList, @$.first_line, "", 1, 2, $1, $2);
}
;
Stmt : Exp SEMI {
    $$ = create_node(Stmt, @$.first_line, "", 0, 2, $1, $2);
}
| CompSt {
    $$ = create_node(Stmt, @$.first_line, "", 1, 1, $1);
}
| RETURN Exp SEMI {
    $$ = create_node(Stmt, @$.first_line, "", 2, 3, $1, $2, $3);
}
| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
    $$ = create_node(Stmt, @$.first_line, "", 3, 5, $1, $2, $3, $4, $5);
}
| IF LP Exp RP Stmt ELSE Stmt {
    $$ = create_node(Stmt, @$.first_line, "", 4, 7, $1, $2, $3, $4, $5, $6, $7);
}
| WHILE LP Exp RP Stmt {
    $$ = create_node(Stmt, @$.first_line, "", 5, 5, $1, $2, $3, $4, $5);
}
| error SEMI
| IF LP Exp RP error ELSE Stmt
;
DefList : /* empty */ {
    $$ = create_node(Epsilon, Epsilon, "", 0, 0);
}
| Def DefList {
    $$ = create_node(DefList, @$.first_line, "", 1, 2, $1, $2);
}
;
Def : Specifier DecList SEMI {
    $$ = create_node(Def, @$.first_line, "", 0, 3, $1, $2, $3);
}
| error SEMI
;
DecList : Dec {
    $$ = create_node(DecList, @$.first_line, "", 0, 1, $1);
}
| Dec COMMA DecList {
    $$ = create_node(DecList, @$.first_line, "", 1, 3, $1, $2, $3);
}
;
Dec : VarDec {
    $$ = create_node(Dec, @$.first_line, "", 0, 1, $1);
}
| VarDec ASSIGNOP Exp {
    $$ = create_node(Dec, @$.first_line, "", 1, 3, $1, $2, $3);
}
;
Exp : Exp ASSIGNOP Exp {
    $$ = create_node(Exp, @$.first_line, "", 0, 3, $1, $2, $3);
}
| Exp AND Exp {
    $$ = create_node(Exp, @$.first_line, "", 1, 3, $1, $2, $3);
}
| Exp OR Exp {
    $$ = create_node(Exp, @$.first_line, "", 2, 3, $1, $2, $3);
}
| Exp RELOP Exp {
    $$ = create_node(Exp, @$.first_line, "", 3, 3, $1, $2, $3);
}
| Exp PLUS Exp {
    $$ = create_node(Exp, @$.first_line, "", 4, 3, $1, $2, $3);
}
| Exp MINUS Exp {
    $$ = create_node(Exp, @$.first_line, "", 5, 3, $1, $2, $3);
}
| Exp STAR Exp {
    $$ = create_node(Exp, @$.first_line, "", 6, 3, $1, $2, $3);
}
| Exp DIV Exp {
    $$ = create_node(Exp, @$.first_line, "", 7, 3, $1, $2, $3);
}
| LP Exp RP {
    $$ = create_node(Exp, @$.first_line, "", 8, 3, $1, $2, $3);
}
| MINUS Exp {
    $$ = create_node(Exp, @$.first_line, "", 9, 2, $1, $2);
}
| NOT Exp {
    $$ = create_node(Exp, @$.first_line, "", 10, 2, $1, $2);
}
| ID LP Args RP {
    $$ = create_node(Exp, @$.first_line, "", 11, 4, $1, $2, $3, $4);
}
| ID LP RP {
    $$ = create_node(Exp, @$.first_line, "", 12, 3, $1, $2, $3);
}
| Exp LB Exp RB {
    $$ = create_node(Exp, @$.first_line, "", 13, 4, $1, $2, $3, $4);
}
| Exp DOT ID {
    $$ = create_node(Exp, @$.first_line, "", 14, 3, $1, $2, $3);
}
| ID {
    $$ = create_node(Exp, @$.first_line, "", 15, 1, $1);
}
| INT {
    $$ = create_node(Exp, @$.first_line, "", 16, 1, $1);
}
| FLOAT {
    $$ = create_node(Exp, @$.first_line, "", 17, 1, $1);
}
| error RP
| error RB
;
Args : Exp COMMA Args {
    $$ = create_node(Args, @$.first_line, "", 0, 3, $1, $2, $3);
}
| Exp {
    $$ = create_node(Args, @$.first_line, "", 1, 1, $1);
}
;

%%

#include "lex.yy.c"

yyerror(char* msg) {
    is_error = 1;
    printf("Error type B at Line %d: syntax error.\n", yylineno);
}

int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);
    yyparse();
    if (!is_error) {
        Program_Node(syntax_root);
        print_inter_code(argv[2]);
        // print_asm(argv[2]);
    }
    return 0;
}

#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "inter_code.h"

enum non_terminals {
    Program = 286, 
    ExtDefList, 
    ExtDef, 
    ExtDecList, 
    Specifier, 
    StructSpecifier, 
    OptTag, 
    Tag, 
    VarDec, 
    FunDec, 
    VarList, 
    ParamDec, 
    CompSt, 
    StmtList, 
    Stmt, 
    DefList, 
    Def, 
    DecList, 
    Dec, 
    Exp, 
    Args,
    Epsilon,
    LOAD,
    STORE,
    DEC,
    ADDRESS,
    PARAM,
    ARG,
    CALL,
    READ,
    WRITE,
    GOTO,
    GE,
    LE,
    GEQ,
    LEQ,
    EQ,
    NEQ
};

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_ {
    enum { FUNC, BASIC, ARRAY, STRUCTURE  } kind;
    union {
        // 基本类型
        int basic;
        // 数组类型
        struct { Type elem; int size; } array;
        // 结构体类型
        FieldList structure;
        // 函数类型
        struct { Type ret_type; FieldList params; int line_no; } func;
    } u;
    int size;
};  

struct FieldList_ {
    char* name; 
    Operand op;
    Type type; 
    FieldList tail; 
};

typedef int NODE_TYPE;

struct node;

typedef struct node_list {
    struct node *head;
    struct node *tail;
} node_list;

typedef struct node {
    NODE_TYPE type;
    int line_no;
    char *val;
    int product_no;
    node_list child_nodes;
    struct node *next;
    Type syn;
    Type inh;
} node;

node *create_node(NODE_TYPE node_type, int line_no, const char *val, int product_no, int child_num, ...);
void traverse(node *root, int depth);

#endif
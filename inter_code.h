#ifndef INTER_CODE_H
#define INTER_CODE_H

typedef struct Operand_* Operand;
struct Operand_ {
    enum { VARIABLE, CONSTANT, TEMP, LABEL } kind;
    union {
        int value;
        int no;
    } u;
    int offset;
};

typedef struct InterCode_* InterCode;
struct InterCode_ {
    int kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand op1, op2, op3; } binop;
        struct { Operand op1; } unop;
        struct { char *name; } func;
        struct { Operand left; char *name; } call;
    } u;
    struct InterCode_ *next;
};

void insert_inter_code(int kind, ...);
void print_inter_code(const char *filename);

extern InterCode inter_codes;

#endif
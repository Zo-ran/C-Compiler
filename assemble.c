#include "assemble.h"
#include "inter_code.h"
#include "syntax_tree.h"
#include "syntax.tab.h"
#include <stdio.h>
#include <stdlib.h>

FILE *f = NULL;
int stack_size = 0;
int param_cnt = 0;
int arg_cnt = 0;

#define _FUNC(name) fprintf(f, "%s:\n", name)
#define _LABEL(i) fprintf(f, "label%d:\n", i)
#define _1AC(instr, a) fprintf(f, "    %s %s\n", instr, a)
#define _2AC(instr, a, b) fprintf(f, "    %s %s, %s\n", instr, a, b)  
#define _2AC_INT(instr, a, b) fprintf(f, "    %s %s, %d\n", instr, a, b)  
#define _2AC_OFF(instr, a, b, offset) fprintf(f, "    %s %s, %d(%s)\n", instr, a, offset, b)  
#define _3AC(instr, a, b, c)  fprintf(f, "    %s %s, %s, %s\n", instr, a, b, c)
#define _3AC_INT(instr, a, b, c)  fprintf(f, "    %s %s, %s, %d\n", instr, a, b, c)
#define _GOTO(i) fprintf(f, "    j label%d\n", i)
#define _BRANCH(instr, a, b, i) fprintf(f, "    %s %s, %s, label%d\n", instr, a, b, i)
#define _JAL(name) fprintf(f, "    jal %s\n", name)
#define _T0 "$t0"
#define _T1 "$t1"
#define _T2 "$t2"
#define _T3 "$t3"
#define _SP "$sp"
#define _FP "$fp"
#define _RA "$ra"
#define _A0 "$a0"
#define _V0 "$v0"
#define _ZERO "$zero"
#define WARNING fprintf(stderr, "warning: usage of uninitialized memory\n")

void init() {
    fprintf(f, ".data\n");
    fprintf(f, "_prompt: .asciiz \"Enter an integer: \"\n");
    fprintf(f, "_ret: .asciiz \"\\n\"\n");
    fprintf(f, ".globl main\n");
    fprintf(f, ".text\n");
    fprintf(f, "read:\n");
    fprintf(f, "    li $v0, 4\n");
    fprintf(f, "    la $a0, _prompt\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    li $v0, 5\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    jr $ra\n\n");
    fprintf(f, "write:\n");
    fprintf(f, "    li $v0, 1\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    li $v0, 4\n");
    fprintf(f, "    la $a0, _ret\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    move $v0, $0\n");
    fprintf(f, "    jr $ra\n\n");
}

void into_stack(Operand op) {
    stack_size += 4;
    op->offset = -stack_size;
}

void print_asm(const char* filename) {
    f = fopen(filename, "w");
    init();
    for (InterCode ic = inter_codes; ic != NULL; ic = ic->next) {
        switch (ic->kind) {
            case LABEL:
                _LABEL(ic->u.unop.op1->u.no);
                break;
            case FUNC:
                param_cnt = 0;
                _FUNC(ic->u.func.name);
                _3AC("addi", _SP, _SP, "-8");
                _2AC_OFF("sw", _RA, _SP, 4);
                _2AC_OFF("sw", _FP, _SP, 0);
                _3AC("addi", _FP, _SP, "8");
                stack_size = 8;
                break;
            case ASSIGNOP: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    if (left->offset == -1) {
                        into_stack(left);
                    }
                    if (right->kind == CONSTANT) {
                        _2AC_INT("li", _T0, right->u.no);
                    } else {
                        if (right->offset == -1) {
                            into_stack(right);
                            WARNING;           
                        }
                        _2AC_OFF("lw", _T0, _FP, right->offset);
                    }
                    _2AC_OFF("sw", _T0, _FP, left->offset);
                }
                break;
            }
            case PLUS: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    if (res->offset == -1) {
                        into_stack(res);
                    }
                    if (op1->offset == -1) {
                        into_stack(op1);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T0, _FP, op1->offset);
                    if (op2->kind == CONSTANT) {
                        _2AC_INT("li", _T1, op2->u.no);
                    } else {
                        if (op2->offset == -1) {
                            into_stack(op2);
                            WARNING;
                        }
                        _2AC_OFF("lw", _T1, _FP, op2->offset);
                    }
                    _3AC("add", _T0, _T0, _T1);
                    _2AC_OFF("sw", _T0, _FP, res->offset);
                }
                break;
            }
            case MINUS: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    if (res->offset == -1) {
                        into_stack(res);
                    }
                    if (op1->kind == CONSTANT) {
                        _2AC_INT("li", _T0, op1->u.no);
                    } else {
                        if (op1->offset == -1) {
                            into_stack(op1);
                            WARNING;
                        }
                        _2AC_OFF("lw", _T0, _FP, op1->offset);
                    }
                    if (op2->offset == -1) {
                        into_stack(op2);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T1, _FP, op2->offset);
                    _3AC("sub", _T0, _T0, _T1);
                    _2AC_OFF("sw", _T0, _FP, res->offset);
                }
                break;
            }
            case STAR: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    if (res->offset == -1) {
                        into_stack(res);
                    }
                    if (op1->offset == -1) {
                        into_stack(op1);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T0, _FP, op1->offset);
                    if (op2->kind == CONSTANT) {
                        _2AC_INT("li", _T1, op2->u.no);
                    } else {
                        if (op2->offset == -1) {
                            into_stack(op2);
                            WARNING;
                        }
                        _2AC_OFF("lw", _T1, _FP, op2->offset);
                    }
                    _3AC("mul", _T0, _T0, _T1);
                    _2AC_OFF("sw", _T0, _FP, res->offset);
                }
                break;
            }
            case DIV: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    if (res->offset == -1) {
                        into_stack(res);
                    }
                    if (op1->offset == -1) {
                        into_stack(op1);
                        WARNING;
                    }
                    if (op2->offset == -1) {
                        into_stack(op2);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T0, _FP, op1->offset);
                    _2AC_OFF("lw", _T1, _FP, op2->offset);
                    _2AC("div", _T0, _T1);
                    _1AC("mflo", _T2);
                    _2AC_OFF("sw", _T2, _FP, res->offset);
                }
                break;
            }
            case ADDRESS: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    if (right->offset == -1) {
                        into_stack(right);
                        WARNING;
                    }
                    if (left->offset == -1) {
                        into_stack(left);
                    }
                    _3AC_INT("addi", _T0, _FP, right->offset);
                    _2AC_OFF("sw", _T0, _FP, left->offset);
                }
                break;
            }
            case LOAD: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    if (right->offset == -1) {
                        into_stack(right);
                        WARNING;
                    }
                    if (left->offset == -1) {
                        into_stack(left);
                    }
                    _2AC_OFF("lw", _T0, _FP, right->offset);
                    _2AC_OFF("lw", _T0, _T0, 0);
                    _2AC_OFF("sw", _T0, _FP, left->offset);
                }
                break;
            }
            case STORE: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    if (left->offset == -1) {
                        into_stack(left);
                        WARNING;
                    }
                    if (right->offset == -1) {
                        into_stack(right);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T0, _FP, left->offset);
                    _2AC_OFF("lw", _T1, _FP, right->offset);
                    _2AC_OFF("sw", _T1, _T0, 0);
                }
                break;
            }
            case GOTO: 
                _GOTO(ic->u.unop.op1->u.no);
                break;
            case GE: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                if (op2->offset == -1) {
                    into_stack(op2);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                _2AC_OFF("lw", _T1, _FP, op2->offset);
                _BRANCH("bgt", _T0, _T1, label->u.no);
                break;
            }
            case GEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                if (op2->offset == -1) {
                    into_stack(op2);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                _2AC_OFF("lw", _T1, _FP, op2->offset);
                _BRANCH("bge", _T0, _T1, label->u.no);
                break;
            }
            case LE: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                if (op2->offset == -1) {
                    into_stack(op2);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                _2AC_OFF("lw", _T1, _FP, op2->offset);
                _BRANCH("blt", _T0, _T1, label->u.no);
                break;
            }
            case LEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                if (op2->offset == -1) {
                    into_stack(op2);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                _2AC_OFF("lw", _T1, _FP, op2->offset);
                _BRANCH("ble", _T0, _T1, label->u.no);
                break;
            }
            case NEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                if (op2->kind == CONSTANT) {
                    _BRANCH("bne", _T0, _ZERO, label->u.no);
                } else {
                    if (op2->offset == -1) {
                        into_stack(op2);
                        WARNING;
                    }
                    _2AC_OFF("lw", _T1, _FP, op2->offset);
                    _BRANCH("bne", _T0, _T1, label->u.no);
                }
                break;
            }
            case EQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1->offset == -1) {
                    into_stack(op1);
                    WARNING;
                }
                if (op2->offset == -1) {
                    into_stack(op2);
                    WARNING;
                }
                _2AC_OFF("lw", _T0, _FP, op1->offset);
                _2AC_OFF("lw", _T1, _FP, op2->offset);
                _BRANCH("beq", _T0, _T1, label->u.no);
                break;
            }
            case RETURN: {
                Operand op = ic->u.unop.op1;
                if (op->offset == -1) {
                    into_stack(op);
                    WARNING;
                }
                _2AC_OFF("lw", _V0, _FP, op->offset);
                _2AC("move", _SP, _FP);
                _2AC("lw", _RA, "-4($sp)");
                _2AC("lw", _FP, "-8($sp)");
                _1AC("jr", _RA);
                fprintf(f, "\n");
                break;
            }
            case DEC: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                _3AC_INT("addi", _SP, _SP, -right->u.no);
                stack_size += right->u.no;
                left->offset = -(stack_size);
                break;
            }
            case ARG: {
                Operand op = ic->u.unop.op1;
                if (arg_cnt == 0) {
                    _3AC_INT("addi", _SP, _FP, -stack_size);
                }
                arg_cnt += 1;
                if (op->offset == -1) {
                    into_stack(op);
                    WARNING;
                } else {
                    _2AC_OFF("lw", _T0, _FP, op->offset);
                    _3AC("addi", _SP, _SP, "-4");
                    _2AC_OFF("sw", _T0, _SP, 0);
                    stack_size += 4;
                }
                break;
            }
            case PARAM: {
                Operand op = ic->u.unop.op1;
                op->offset = param_cnt * 4;
                param_cnt += 1;
                break;
            }
            case READ: {
                Operand op = ic->u.unop.op1;
                _JAL("read");
                if (op->offset == -1) {
                    into_stack(op);
                }
                _2AC_OFF("sw", _V0, _FP, op->offset);
                break;
            }
            case CALL: {
                arg_cnt = 0;
                Operand left = ic->u.call.left;
                _JAL(ic->u.call.name);
                if (left->offset == -1) {
                    into_stack(left);
                }
                _2AC_OFF("sw", _V0, _FP, left->offset);
                break;
            }
            case WRITE: {
                Operand op = ic->u.unop.op1;
                if (op->offset == -1) {
                    into_stack(op);
                    WARNING;
                }
                _2AC_OFF("lw", _A0, _FP, op->offset);
                _JAL("write");
                break;
            }
        }
    }
    fclose(f);
}
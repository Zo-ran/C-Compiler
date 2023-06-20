#include "inter_code.h"
#include "syntax_tree.h"
#include "syntax.tab.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

InterCode inter_codes = NULL;
InterCode tail = NULL;

void insert_inter_code(int kind, ...) {
    InterCode ic = malloc(sizeof(struct InterCode_));
    ic->kind = kind;
    va_list p_args;
    va_start(p_args, kind);
    switch (kind) {
        case FUNC:
            ic->u.func.name = va_arg(p_args, char *);
            break;
        case CALL:
            ic->u.call.left = va_arg(p_args, Operand);
            ic->u.call.name =  va_arg(p_args, char *);
            break;
        case RETURN:
        case PARAM:
        case ARG:
        case READ:
        case WRITE:
        case LABEL:
        case GOTO:
            ic->u.unop.op1 = va_arg(p_args, Operand);
            break;
        case ASSIGNOP:
        case LOAD:
        case STORE:
        case ADDRESS:
        case DEC:
            ic->u.assign.left = va_arg(p_args, Operand);
            ic->u.assign.right = va_arg(p_args, Operand);
            break;
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
        case GE:
        case LE:
        case GEQ:
        case LEQ:
        case EQ:
        case NEQ:
            ic->u.binop.op1 = va_arg(p_args, Operand);
            ic->u.binop.op2 = va_arg(p_args, Operand);
            ic->u.binop.op3 = va_arg(p_args, Operand);
            break;
        default:
            exit(-1);
    }
    if (inter_codes == NULL) {
        inter_codes = ic;
    } else {
        tail->next = ic;
    }
    tail = ic;
}

char get_operand(Operand op) {
    switch (op->kind) {
        case VARIABLE:
            return 'v';
        case CONSTANT:
            return '#';
        case TEMP:
            return 't';
        case LABEL:
            return 'l';
        default:
            return 'e';
    }
}

void print_inter_code(const char *filename) {
    FILE *f = fopen(filename, "w");
    for (InterCode ic = inter_codes; ic != NULL; ic = ic->next) {
        switch (ic->kind) {
            case LABEL:
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "LABEL %c%d :\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            case FUNC:
                if (ic->u.func.name != NULL) {
                    fprintf(f, "FUNCTION %s :\n", ic->u.func.name);
                }
                break;
            case ASSIGNOP: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    fprintf(f, "%c%d := %c%d\n", get_operand(left), left->u.no, get_operand(right), right->u.no);
                }
                break;
            }
            case PLUS: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    fprintf(f, "%c%d := %c%d + %c%d\n", get_operand(res), res->u.no, get_operand(op1), op1->u.no, get_operand(op2), op2->u.no);
                }
                break;
            }
            case MINUS: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    fprintf(f, "%c%d := %c%d - %c%d\n", get_operand(res), res->u.no, get_operand(op1), op1->u.no, get_operand(op2), op2->u.no);
                }
                break;
            }
            case STAR: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    fprintf(f, "%c%d := %c%d * %c%d\n", get_operand(res), res->u.no, get_operand(op1), op1->u.no, get_operand(op2), op2->u.no);
                }
                break;
            }
            case DIV: {
                Operand res = ic->u.binop.op1;
                Operand op1 = ic->u.binop.op2;
                Operand op2 = ic->u.binop.op3;
                if (res != NULL && op1 != NULL && op2 != NULL) {
                    fprintf(f, "%c%d := %c%d / %c%d\n", get_operand(res), res->u.no, get_operand(op1), op1->u.no, get_operand(op2), op2->u.no);
                }
                break;
            }
            case ADDRESS: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    fprintf(f, "%c%d := &%c%d\n", get_operand(left), left->u.no, get_operand(right), right->u.no);
                }
                break;
            }
            case LOAD: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    fprintf(f, "%c%d := *%c%d\n", get_operand(left), left->u.no, get_operand(right), right->u.no);
                }
                break;
            }
            case STORE: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    fprintf(f, "*%c%d := %c%d\n", get_operand(left), left->u.no, get_operand(right), right->u.no);
                }
                break;
            }
            case GOTO: {
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "GOTO %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            }
            case GE: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d > %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case GEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d >= %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case LE: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d < %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case LEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d <= %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case NEQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d != %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case EQ: {
                Operand op1 = ic->u.binop.op1;
                Operand op2 = ic->u.binop.op2;
                Operand label = ic->u.binop.op3;
                if (op1 != NULL && op2 != NULL && label != NULL) {
                    fprintf(f, "IF %c%d == %c%d GOTO %c%d\n", get_operand(op1), op1->u.no, get_operand(op2), op2->u.no, get_operand(label), label->u.no);
                }
                break;
            }
            case RETURN:
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "RETURN %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            case DEC: {
                Operand left = ic->u.assign.left;
                Operand right = ic->u.assign.right;
                if (left != NULL && right != NULL) {
                    fprintf(f, "DEC %c%d %d\n", get_operand(left), left->u.no, right->u.no);
                }
                break;
            }
            case ARG: {
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "ARG %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            }
            case PARAM: {
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "PARAM %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            }
            case READ: {
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "READ %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            }
            case CALL: {
                Operand left = ic->u.call.left;
                char *name = ic->u.call.name;
                if (left != NULL && name != NULL) {
                    fprintf(f, "%c%d := CALL %s\n", get_operand(left), left->u.no, name);
                }
                break;
            }
            case WRITE: {
                if (ic->u.unop.op1 != NULL) {
                    fprintf(f, "WRITE %c%d\n", get_operand(ic->u.unop.op1), ic->u.unop.op1->u.no);
                }
                break;
            }
        }
    }
    fclose(f);
}
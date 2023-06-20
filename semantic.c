#include "semantic.h"
#include "hashmap.h"
#include "syntax.tab.h"
#include "syntax_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <stdarg.h>

int cnt_temp = 0;
int cnt_variable = 0;
int cnt_label = 0;
Operand zero;
Operand one;

Operand new_temp() {
    Operand op = malloc(sizeof(struct Operand_));
    op->kind = TEMP;
    op->u.no = ++cnt_temp;
    op->offset = -1;
    return op;
}

Operand new_variable() {
    Operand op = malloc(sizeof(struct Operand_));
    op->kind = VARIABLE;
    op->u.no = ++cnt_variable;
    op->offset = -1;
    return op;
}

Operand new_constant(int val) {
    Operand op = malloc(sizeof(struct Operand_));
    op->kind = CONSTANT;
    op->u.value = val;
    op->offset = -1;
    return op;
}

Operand new_label() {
    Operand op = malloc(sizeof(struct Operand_));
    op->kind = LABEL;
    op->u.no = ++cnt_label;
    op->offset = -1;
    return op;
}

Type new_int() {
    Type t = (Type)malloc(sizeof(struct Type_));
    t->kind = BASIC;
    t->u.basic = INT;
    t->size = 4;
    return t;
}

int get_relop(const char *str) {
    if (strcmp(str, ">") == 0) {
        return GE;
    } else if (strcmp(str, "<") == 0) {
        return LE;
    } else if (strcmp(str, ">=") == 0) {
        return GEQ;
    } else if (strcmp(str, "<=") == 0) {
        return LEQ;
    } else if (strcmp(str, "==") == 0) {
        return EQ;
    } else if (strcmp(str, "!=") == 0) {
        return NEQ;
    } else {
        assert(0);
        return 0;
    }
}

void array_error() {
    printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
    exit(0);
}

void Program_Node(node *n) {
    one = new_constant(1);
    zero = new_constant(0);
    ExtDefList_Node(n->child_nodes.head);
}

void ExtDefList_Node(node *n) {
    if (n->product_no == 1) {
        node *curr = n->child_nodes.head;
        ExtDef_Node(curr);
        ExtDefList_Node(curr->next);
    }
}

void ExtDef_Node(node *n) {
    node *specifier = n->child_nodes.head;
    Specifier_Node(specifier);
    if (n->product_no == 0) {
        node *extdecList = specifier->next;
        extdecList->inh = specifier->syn;
        ExtDecList_Node(extdecList);
    } else if (n->product_no == 2) {
        node *fundec = specifier->next;
        node *compst = fundec->next;
        fundec->inh = specifier->syn;
        FunDec_Node(fundec);
        compst->inh = specifier->syn;
        CompSt_Node(compst);
    }
}

void FunDec_Node(node *n) {
    n->syn = (Type)malloc(sizeof(struct Type_));
    n->syn->kind = FUNC;
    n->syn->size = n->inh->size;
    n->syn->u.func.ret_type = n->inh;
    n->syn->u.func.params = NULL;
    n->syn->u.func.line_no = n->line_no;
    char *func_name = n->child_nodes.head->val;
    insert_inter_code(FUNC, func_name);
    if (n->product_no == 0) {
        node *varlist = n->child_nodes.head->next->next;
        VarList_Node(varlist);
        n->syn->u.func.params = varlist->syn->u.structure;
    } 
    insert(func_name, n->syn, NULL);
}

void ExtDecList_Node(node *n) {
    node *vardec = n->child_nodes.head;
    vardec->inh = n->inh;
    VarDec_Node(vardec);
    Operand v = new_variable();
    insert(vardec->val, vardec->syn->u.structure->type, v);
    if (n->product_no == 1) {
        node *extdeclist = vardec->next->next;
        extdeclist->inh = n->inh;
        ExtDecList_Node(extdeclist);
    }
}

void Specifier_Node(node *n) {
    node *head = n->child_nodes.head;
    if (n->product_no == 0) {
        n->syn = (Type)malloc(sizeof(struct Type_));
        n->syn->kind = BASIC;
        n->syn->size = 4;
        if (strcmp(head->val, "int") == 0) {
            n->syn->u.basic = INT;
        } else {
            n->syn->u.basic = FLOAT;
        }
    } else {
        StructSpecifier_Node(head);
        n->syn = head->syn;
    }
}

void StructSpecifier_Node(node *n) {
    if (n->product_no == 0) {
        node *deflist = n->child_nodes.head->next->next->next;
        DefList_Node(deflist, true);
        n->syn = deflist->syn;  
        node *opttag = n->child_nodes.head->next;
        if (opttag->product_no == 1) {
            insert(opttag->child_nodes.head->val, n->syn, NULL);
        }
    } else {
        node *tag = n->child_nodes.head->next;
        char *name = tag->child_nodes.head->val;
        FieldList f = search(name);
        n->syn = f->type;            
    }
}

void DefList_Node(node *n, int from_struct) {
    if (n->product_no == 0) {
        n->syn = (Type)malloc(sizeof(struct Type_));
        n->syn->kind = STRUCTURE;
        n->syn->size = 0;
        n->syn->u.structure = NULL;
        return ;
    }
    node *def = n->child_nodes.head;
    node *deflist = def->next;
    Def_Node(def, from_struct);
    DefList_Node(deflist, from_struct);
    FieldList tail = def->syn->u.structure;
    while (tail->tail != NULL)
        tail = tail->tail;
    tail->tail = deflist->syn->u.structure;
    def->syn->size += deflist->syn->size;
    n->syn = def->syn;
}

void Def_Node(node *n, int from_struct) {
    node *specifier = n->child_nodes.head;
    node *declist = specifier->next;
    Specifier_Node(specifier);
    declist->inh = specifier->syn;
    DecList_Node(declist, from_struct);
    n->syn = declist->syn;
}

void DecList_Node(node *n, int from_struct) {
    node *dec = n->child_nodes.head;
    dec->inh = n->inh;
    Dec_Node(dec, from_struct);
    if (n->product_no == 0) {
        dec->syn->u.structure->tail = NULL;
    } else {
        node *declist = dec->next->next;
        declist->inh = n->inh;
        DecList_Node(declist, from_struct);
        dec->syn->u.structure->tail = declist->syn->u.structure;
        dec->syn->size += declist->syn->size;
    }
    n->syn = dec->syn;
}

void Dec_Node(node *n, int from_struct) {
    node *vardec = n->child_nodes.head;
    vardec->inh = n->inh;
    VarDec_Node(vardec);
    n->syn = vardec->syn;
    n->syn->size = vardec->syn->u.structure->type->size;
    if (!from_struct) {
        Operand v = new_variable();
        if (n->syn->u.structure->type->kind != BASIC) {            
            Operand t = new_temp();
            insert_inter_code(DEC, t, new_constant(n->syn->u.structure->type->size));
            insert_inter_code(ADDRESS, v, t);
        }
        insert(vardec->val, n->syn->u.structure->type, v);
        if (n->product_no == 1) {
            Operand t = new_temp();
            node *exp_node = vardec->next->next;
            Exp_Node(exp_node, t, false);
            insert_inter_code(ASSIGNOP, v, t);
        }
    }
}

void VarDec_Node(node *n) {
    n->syn = (Type)malloc(sizeof(struct Type_));
    n->syn->u.structure = (FieldList)malloc(sizeof(struct FieldList_));
    n->syn->kind = STRUCTURE;
    n->syn->u.structure->tail = NULL;
    if (n->product_no == 0) {
        n->val = n->child_nodes.head->val;
        n->syn->u.structure->type = n->inh;
    } else {
        node *vardec = n->child_nodes.head;
        vardec->inh = n->inh;
        if (vardec->product_no != 0) {
            array_error();
        }
        VarDec_Node(vardec);
        n->val = vardec->val;
        n->syn->u.structure->type = (Type)malloc(sizeof(struct Type_));
        n->syn->u.structure->type->kind = ARRAY;
        n->syn->u.structure->type->size = atoi(vardec->next->next->val) * vardec->syn->u.structure->type->size;
        n->syn->u.structure->type->u.array.elem = vardec->syn->u.structure->type;
        n->syn->u.structure->type->u.array.size = atoi(vardec->next->next->val);
    }
    n->syn->u.structure->name = n->val;
}

void VarList_Node(node *n) {
    node *paramdec = n->child_nodes.head;
    ParamDec_Node(paramdec);
    if (n->product_no == 0) {
        node *varlist = paramdec->next->next;
        VarList_Node(varlist);
        paramdec->syn->u.structure->tail = varlist->syn->u.structure;
    }
    n->syn = paramdec->syn;
}

void ParamDec_Node(node *n) {
    node *specifier = n->child_nodes.head;
    node *vardec = specifier->next;
    Specifier_Node(specifier);
    vardec->inh = specifier->syn;
    VarDec_Node(vardec);
    Operand v = new_variable();
    if (vardec->syn->u.structure->type->kind == ARRAY) {
        array_error();
    }
    insert(vardec->val, vardec->syn->u.structure->type, v);
    insert_inter_code(PARAM, v);
    n->syn = vardec->syn;
}

void CompSt_Node(node *n) {
    DefList_Node(n->child_nodes.head->next, false);
    n->child_nodes.head->next->next->inh = n->inh;
    StmtList_Node(n->child_nodes.head->next->next);
}

void StmtList_Node(node *n) {
    if (n->product_no == 1) {
        node *stmt_node = n->child_nodes.head;
        node *stmtlist_node = n->child_nodes.head->next;
        stmt_node->inh = n->inh;
        stmtlist_node->inh = n->inh;
        Stmt_Node(stmt_node);
        StmtList_Node(stmtlist_node);
    }
}

void Stmt_Node(node *n) {
    switch(n->product_no) {
        case 0:
            Exp_Node(n->child_nodes.head, NULL, false);
            break;
        case 1:
            n->child_nodes.head->inh = n->inh;
            CompSt_Node(n->child_nodes.head);
            break;
        case 2: {
            Operand t = new_temp();
            Exp_Node(n->child_nodes.head->next, t, false);
            insert_inter_code(RETURN, t);
            break;
        }
        case 3: {
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand t = new_variable();
            Exp_Node(n->child_nodes.head->next->next, t, true, label1, label2);
            n->child_nodes.head->next->next->next->next->inh = n->inh;
            insert_inter_code(LABEL, label1);
            Stmt_Node(n->child_nodes.head->next->next->next->next);
            insert_inter_code(LABEL, label2);
            break;
        }
        case 4: {
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            Operand t = new_variable();
            Exp_Node(n->child_nodes.head->next->next, t, true, label1, label2);
            n->child_nodes.head->next->next->next->next->inh = n->inh;
            n->child_nodes.head->next->next->next->next->next->next->inh = n->inh;
            insert_inter_code(LABEL, label1);
            Stmt_Node(n->child_nodes.head->next->next->next->next);
            insert_inter_code(GOTO, label3);
            insert_inter_code(LABEL, label2);
            Stmt_Node(n->child_nodes.head->next->next->next->next->next->next);
            insert_inter_code(LABEL, label3);
            break;
        }
        case 5: {
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            Operand t = new_variable();
            insert_inter_code(LABEL, label1);
            Exp_Node(n->child_nodes.head->next->next, t, true, label2, label3);
            n->child_nodes.head->next->next->next->next->inh = n->inh;
            insert_inter_code(LABEL, label2);
            Stmt_Node(n->child_nodes.head->next->next->next->next);
            insert_inter_code(GOTO, label1);
            insert_inter_code(LABEL, label3);
            break;
        }
        default:
            assert(0);
    }
}

void Exp_Node(node *n, Operand place, int jump, ...) {
    FieldList f;
    node *head = n->child_nodes.head, *exp1, *exp2;
    switch (n->product_no) {
        case 0: {
            Operand t1;
            Operand t2 = new_temp();
            exp1 = head;
            exp2 = head->next->next;
            Exp_Node(exp2, t2, false);
            if (exp1->product_no == 13) {
                exp1 = exp1->child_nodes.head;
                exp2 = exp1->next->next;
                if (exp1->product_no == 13) {
                    array_error();
                } else {
                    Operand t11 = new_temp();
                    Operand t22 = new_temp();
                    Operand t33 = new_temp();
                    Operand t44 = new_temp();
                    Exp_Node(exp1, t11, false);
                    Exp_Node(exp2, t22, false);
                    n->syn = exp1->syn->u.array.elem;
                    insert_inter_code(STAR, t33, t22, new_constant(n->syn->size));
                    insert_inter_code(PLUS, t44, t11, t33);
                    insert_inter_code(STORE, t44, t2);
                }
            } else if (exp1->product_no == 14) {
                Operand t11 = new_temp();
                Operand t22 = new_temp();
                node *head = exp1->child_nodes.head;
                Exp_Node(head, t11, false);
                f = head->syn->u.structure;
                int offset = 0;
                while (strcmp(f->name, head->next->next->val) != 0) {
                    offset += f->type->size;
                    f = f->tail;
                }
                insert_inter_code(PLUS, t22, t11, new_constant(offset));
                insert_inter_code(STORE, t22, t2);
            } else if (exp1->product_no == 15) {
                FieldList f = search(exp1->child_nodes.head->val);
                t1 = f->op;
                insert_inter_code(ASSIGNOP, t1, t2);
            }
            insert_inter_code(ASSIGNOP, place, t2);
            n->syn = exp2->syn;
            break;
        }
        case 1: {
            exp1 = head;
            exp2 = head->next->next;
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            Operand t1 = new_variable();
            insert_inter_code(ASSIGNOP, place, zero);
            Exp_Node(exp1, t1, true, label1, label3);
            insert_inter_code(LABEL, label1);
            Exp_Node(exp2, t1, true, label2, label3);
            insert_inter_code(LABEL, label2);
            insert_inter_code(ASSIGNOP, place, one);
            insert_inter_code(LABEL, label3);
            n->syn = new_int();
            break;
        }
        case 2: {
            exp1 = head;
            exp2 = head->next->next;
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            Operand t1 = new_variable();
            insert_inter_code(ASSIGNOP, place, one);
            Exp_Node(exp1, t1, true, label3, label2);
            insert_inter_code(LABEL, label2);
            Exp_Node(exp2, t1, true, label3, label1);
            insert_inter_code(LABEL, label1);
            insert_inter_code(ASSIGNOP, place, zero);
            insert_inter_code(LABEL, label3);
            n->syn = new_int();
            break;
        }
        case 3: {
            exp1 = head;
            exp2 = head->next->next;
            Operand t1 = new_temp();
            Operand t2 = new_temp();    
            Operand label1 = new_label();
            int relop = get_relop(head->next->val);
            Exp_Node(exp1, t1, false);
            Exp_Node(exp2, t2, false);
            insert_inter_code(ASSIGNOP, place, one);
            insert_inter_code(relop, t1, t2, label1);
            insert_inter_code(ASSIGNOP, place, zero);
            insert_inter_code(LABEL, label1);
            n->syn = new_int();
            break;
        }
        case 4: 
        case 5:
        case 6:
        case 7: {
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            exp1 = head;
            exp2 = head->next->next;
            Exp_Node(exp1, t1, false);
            Exp_Node(exp2, t2, false);
            insert_inter_code(PLUS + n->product_no - 4, place, t1, t2);
            n->syn = exp1->syn;
            break;
        }
        case 8: {
            Exp_Node(head->next, place, false);
            n->syn = head->next->syn;
            break;
        }
        case 9: {
            Operand t1 = new_temp();
            Exp_Node(head->next, t1, false);
            insert_inter_code(MINUS, place, zero, t1);
            n->syn = head->next->syn;
            break;
        }
        case 10: {
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand t = new_variable();
            insert_inter_code(ASSIGNOP, place, zero);
            Exp_Node(head->next, t, true, label1, label2);
            insert_inter_code(LABEL, label2);
            insert_inter_code(ASSIGNOP, place, one);
            insert_inter_code(LABEL, label1);
            n->syn = head->next->syn;
            break;
        }
        case 11:
        case 12:
            if (strcmp(head->val, "read") == 0) {
                n->syn = new_int();
                insert_inter_code(READ, place);
            } else if (strcmp(head->val, "write") == 0) {
                Operand t = new_temp();
                Exp_Node(head->next->next->child_nodes.head, t, false);
                insert_inter_code(WRITE, t);
                insert_inter_code(ASSIGNOP, place, zero);
            } else {
                f = search(head->val);
                n->syn = f->type->u.func.ret_type;
                if (n->product_no == 11) {
                    FieldList arg_list = Args_Node(head->next->next, NULL);
                    for (FieldList arg = arg_list; arg != NULL; arg = arg->tail) {
                        insert_inter_code(ARG, arg->op);
                    }
                }
                insert_inter_code(CALL, place == NULL ? new_temp() : place, head->val);
            }
            break;
        case 13:
            exp1 = head;
            exp2 = head->next->next;
            if (exp1->product_no == 13) {
                array_error();
            } else {
                Operand t1 = new_temp();
                Operand t2 = new_temp();
                Operand t3 = new_temp();
                Exp_Node(exp1, t1, false);
                Exp_Node(exp2, t2, false);
                n->syn = exp1->syn->u.array.elem;
                insert_inter_code(STAR, t3, t2, new_constant(n->syn->size));
                insert_inter_code(PLUS, place, t1, t3);
                if (n->syn->kind == BASIC) {
                    insert_inter_code(LOAD, place, place);
                }
            }
            break;
        case 14: {
            Operand t1 = new_temp();
            Exp_Node(head, t1, false);
            f = head->syn->u.structure;
            int offset = 0;
            while (strcmp(f->name, head->next->next->val) != 0) {
                offset += f->type->size;
                f = f->tail;
            }
            insert_inter_code(PLUS, place, t1, new_constant(offset));
            if (f->type->kind == BASIC) {
                insert_inter_code(LOAD, place, place);
            }
            n->syn = f->type;
            break;
        }
        case 15:
            f = search(head->val);
            insert_inter_code(ASSIGNOP, place, f->op);
            n->val = head->val;
            n->syn = f->type;
            break;
        case 16:
            insert_inter_code(ASSIGNOP, place, new_constant(atoi(n->child_nodes.head->val)));
            n->syn = new_int();
            break;
        case 17:
            assert(0);
        default:
            assert(0);
    }
    if (jump) {
        va_list p_args;
        va_start(p_args, jump);
        Operand label_true = va_arg(p_args, Operand);
        Operand label_false = va_arg(p_args, Operand);
        insert_inter_code(NEQ, place, zero, label_true);
        insert_inter_code(GOTO, label_false);
    }
}   

FieldList Args_Node(node *n, FieldList tail) {
    node *exp_node = n->child_nodes.head;
    Operand t = new_temp();
    Exp_Node(exp_node, t, false);
    FieldList arg = (FieldList)malloc(sizeof(struct FieldList_));
    arg->op = t;
    arg->tail = tail;
    if (n->product_no == 0) {
        node *args_node = exp_node->next->next;
        arg = Args_Node(args_node, arg);
    }
    return arg;
}

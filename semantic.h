#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "syntax_tree.h"
#include "inter_code.h"

void Program_Node(node *n);
void ExtDefList_Node(node *n);
void ExtDef_Node(node *n);
void ExtDecList_Node(node *n);
void Specifier_Node(node *n);
void StructSpecifier_Node(node *n);
void OptTag_Node(node *n);
void Tag_Node(node *n);
void VarDec_Node(node *n);
void FunDec_Node(node *n);
void VarList_Node(node *n);
void ParamDec_Node(node *n);
void CompSt_Node(node *n);
void StmtList_Node(node *n);
void Stmt_Node(node *n);
void DefList_Node(node *n, int from_struct);
void Def_Node(node *n, int from_struct);
void DecList_Node(node *n, int from_struct);
void Dec_Node(node *n, int from_struct);
void Exp_Node(node *n, Operand place, int jump, ...);
FieldList Args_Node(node *n, FieldList arg_list);

#endif
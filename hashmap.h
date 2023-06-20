#ifndef HASHMAP_H
#define HASHMAP_H

#include "syntax_tree.h"

#define MAX_SIZE 0x3fff + 1
#define false 0
#define true 1

extern FieldList symtab[MAX_SIZE];

int insert(char *n, Type t, Operand op);
FieldList search(char *n);

#endif
#include "hashmap.h"
#include "semantic.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

FieldList symtab[MAX_SIZE];

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~0x3fff) 
            val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

int insert(char *n, Type t, Operand op) {
    unsigned int idx = hash_pjw(n);
    assert(idx >= 0 && idx < MAX_SIZE);
    for (FieldList i = symtab[idx]; i != NULL; i = i->tail) {
        if (strcmp(i->name, n) == 0)   
            return false;
    }
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    f->name = n;
    f->type = t;
    f->tail = symtab[idx];
    f->op = op;
    symtab[idx] = f;
    return true;
}

FieldList search(char *n) {
    unsigned int idx =  hash_pjw(n);
    assert(idx >= 0 && idx < MAX_SIZE);
    for (FieldList i = symtab[idx]; i != NULL; i = i->tail) {
        if (strcmp(i->name, n) == 0)   
            return i;
    }
    return NULL;
}
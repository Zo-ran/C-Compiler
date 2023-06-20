#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "syntax_tree.h"
#include "syntax.tab.h"

void add_child_node(node *father, node *child) {
    if (father->child_nodes.head == NULL)
        father->child_nodes.head = child;
    else
        father->child_nodes.tail->next = child;
    father->child_nodes.tail = child;
}

node *create_node(NODE_TYPE type, int line_no, const char *val, int product_no, int child_num, ...) {
    node *res = (node *)malloc(sizeof(node));
    res->type = type;
    res->line_no = line_no;
    res->val = (char *)malloc(strlen(val) + 1);
    res->product_no = product_no;
    res->child_nodes.head = NULL;
    res->child_nodes.tail = NULL;
    res->next = NULL;
    res->inh = NULL;
    res->syn = NULL;
    strcpy(res->val, val);
    va_list p_args;
    va_start(p_args, child_num);
    for (int i = 0; i < child_num; ++i) {
        node *child = va_arg(p_args, node *);
        add_child_node(res, child);
    }
    return res;
}

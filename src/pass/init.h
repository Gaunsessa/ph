#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <ht.h>
#include <dynarr.h>
#include <print.h>

#include "util.h"
#include "lexer.h"
#include "node.h"
#include "symbol.h"

void init_pass(node_t *AST, sym_table_t *tbl);

bool init_special(node_t *node);
void init_start(node_t *node, sym_table_t *tbl, size_t scope);
void init_end(node_t *node, sym_table_t *tbl, size_t scope);

#endif
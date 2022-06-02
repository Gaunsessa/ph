#ifndef DESUGAR_H
#define DESUGAR_H

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
#include "types.h"

void desugar_pass(node_t *AST, sym_table_t *tbl);

bool desugar_special(node_t *node);
void desugar_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void desugar_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);

#endif
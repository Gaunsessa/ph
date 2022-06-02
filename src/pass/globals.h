#ifndef GLOBALS_H
#define GLOBALS_H

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
#include "infer.h"

void globals_pass(node_t *AST, sym_table_t *tbl);

bool globals_special(node_t *node);
void globals_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void globals_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);

#endif
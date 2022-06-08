#ifndef CHECKER_H
#define CHECKER_H

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

typedef struct ckr_scope_t {
   dynarr_t(wchar_t *) decls;
   type_idx ret;
} ckr_scope_t;

typedef struct checker_t {
   dynarr_t(ckr_scope_t *) scopes;

   size_t errors;
} checker_t;

void checker_pass(node_t *AST, sym_table_t *tbl);

bool checker_special(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope);
void checker_start(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void checker_end(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope);

bool checker_scope_exists(checker_t *ckr, size_t scope, wchar_t *name);

#define NODE(ident, ...) void checker_##ident(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope);
   NODE_TYPES
#undef NODE

#endif
#ifndef TYPERES_H
#define TYPERES_H

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

void typeres_pass(node_t *AST, sym_table_t *tbl);

bool typeres_special(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope);
void typeres_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void typeres_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope);

void typeres_typebase(node_t *btyp, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void typeres_vardecl(node_t *vdecl, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void typeres_funcdecl(node_t *vdecl, sym_table_t *tbl, sym_module_t *mod, size_t scope);
void typeres_impl(node_t *impl, sym_table_t *tbl, sym_module_t *mod, size_t scope);

type_idx typeres_resolve_type(type_idx idx, sym_table_t *tbl, sym_module_t *mod, size_t scope);

#endif
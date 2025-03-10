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

typedef struct desugar_t {
   wchar_t *module_name;
   size_t module_name_len;

   node_t *AST;

   dynarr_t(node_t *) parent_scopes;
   node_t * parent_scope;
} desugar_t;

node_t *desugar_desugar(node_t *AST);

bool desugar_special(node_t *node);
void desugar_start(node_t *node, desugar_t *des);
void desugar_end(node_t *node, desugar_t *des);

void desugar_file(node_t *file, desugar_t *des);

void desugar_var_decl(node_t *vard, desugar_t *des);

void desugar_ident(node_t *ident, desugar_t *des);

void desugar_feild_expr(node_t *expr, desugar_t *des);
void desugar_method_expr(node_t *expr, desugar_t *des);

void desugar_data_type(node_t *dtype, desugar_t *des);
void desugar_type(type_t *type, desugar_t *des);

wchar_t *desugar_rename_ident2(wchar_t *ident, desugar_t *des);
#define desugar_rename_ident_static(des, ident, ...) desugar_rename_ident(des, ident, M_VALEN(__VA_ARGS__), (wchar_t *[]) { ##__VA_ARGS__ })
wchar_t *desugar_rename_ident(desugar_t *des, wchar_t *ident, size_t amt, wchar_t **args);

type_t *desugar_resolve_type(type_t *type);

#endif
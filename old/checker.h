#ifndef CHECKER_H
#define CHECKER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <dynarr.h>
#include <print.h>

#include "util.h"
#include "lexer.h"
#include "node.h"
#include "prims.h"
#include "error.h"

typedef struct scope_t {
   ht_t(wchar_t *, type_idx) decls;
   ht_t(wchar_t *, type_idx) types;
   type_idx ret;

   struct scope_t *parent;
} scope_t;

typedef struct module_t {
   scope_t *scope;

   scope_t *cur_scope;
   scope_t *file_scope;
} module_t;

typedef struct checker_t {
   type_handler_t *type_handler;
   ht_t(wchar_t *, module_t *) modules;

   bool error;
   size_t errors;
} checker_t;

// Checker
bool checker_check(node_t *AST, type_handler_t *type_handler);
bool checker_check_special(node_t *node);
void checker_check_start(node_t *node, checker_t *ckr, module_t *mod);
void checker_check_end(node_t *node, checker_t *ckr, module_t *mod);

bool checker_check_node(checker_t *ckr, module_t *mod, node_t *node);

scope_t *checker_scope_new();
void checker_scope_free(scope_t *scope);

void checker_push_scope(module_t *mod);
void checker_pop_scope(module_t *mod);

type_idx checker_get_decl(module_t *mod, wchar_t *ident, bool typedf);
type_idx checker_get_decl_both(module_t *mod, wchar_t *ident);
type_idx checker_resolve_base_type(module_t *mod, type_idx idx);

void checker_set_decl(module_t *mod, wchar_t *ident, type_idx idx, bool typedf);
bool checker_decl_exists_cur(module_t *mod, wchar_t *ident);
void free_decl(void *decl);

// Check
bool checker_check_check(checker_t *ckr, node_t *node);

#define NODE(ident, ...) bool checker_check_##ident(checker_t *ckr, module_t *mod, node_t *n);
   NODE_TYPES
#undef NODE

// Infer
type_t *checker_infer_expression(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_expression_no_res(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_binexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_pathexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_feildexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_methodexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_addrexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_derefexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_castexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_callexpr(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_callexpr_funct(checker_t *ckr, module_t *mod, node_t *expr);
type_idx checker_infer_literal(checker_t *ckr, module_t *mod, node_t *lit);
type_idx checker_infer_identifier(checker_t *ckr, module_t *mod, node_t *ident);

type_idx checker_infer_var_decl(checker_t *ckr, module_t *mod, node_t *vard);
type_idx checker_infer_alias(checker_t *ckr, module_t *mod, node_t *alias);

#endif
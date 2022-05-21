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

// typedef struct decl_t {
//    bool is_typedef;
//    bool is_init;

//    type_t *type;
// } decl_t;

typedef struct scope_t {
   ht_t(wchar_t *, type_t *) decls;
   ht_t(wchar_t *, type_t *) types;
   type_t *ret;

   struct scope_t *parent;
} scope_t;

typedef struct checker_t {
   // ht_t(wchar_t *, scope_t *) modules;

   scope_t scope;
   scope_t *cur_scope;
   scope_t *file_scope;

   bool error;
   size_t errors;
} checker_t;

// Checker
bool checker_check(node_t *AST);
bool checker_check_special(node_t *node);
void checker_check_start(node_t *node, checker_t *ckr);
void checker_check_end(node_t *node, checker_t *ckr);

bool checker_check_node(checker_t *ckr, node_t *node);

void checker_push_scope(checker_t *ckr);
void checker_pop_scope(checker_t *ckr);

type_t *checker_get_decl(checker_t *ckr, wchar_t *ident, bool typedf);
type_t *checker_reslove_type(checker_t *ckr, type_t *type);
type_t *checker_reslove_base_type(checker_t *ckr, type_t *type);
type_t *checker_reslove_typedef(checker_t *ckr, type_t *type);

void checker_set_decl(checker_t *ckr, wchar_t *ident, type_t *type, bool typedf);
bool checker_decl_exists_cur(checker_t *ckr, wchar_t *ident);
void free_decl(void *decl);

// Check
bool checker_check_check(checker_t *ckr, node_t *node);

#define NODE(ident, ...) bool checker_check_##ident(checker_t *ckr, node_t *n);
   NODE_TYPES
#undef NODE

// Infer
type_t *checker_infer_expression(checker_t *ckr, node_t *expr);
type_t *checker_infer_expression_no_res(checker_t *ckr, node_t *expr);
type_t *checker_infer_binexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_feildexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_methodexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_addrexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_derefexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_castexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_callexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_callexpr_funct(checker_t *ckr, node_t *expr);
type_t *checker_infer_literal(checker_t *ckr, node_t *lit);
type_t *checker_infer_identifier(checker_t *ckr, node_t *ident);

type_t *checker_infer_var_decl(checker_t *ckr, node_t *vard);
type_t *checker_infer_alias(checker_t *ckr, node_t *alias);

#endif
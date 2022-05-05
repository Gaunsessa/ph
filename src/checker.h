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

typedef struct decl_t {
   bool is_typedef;
   bool is_init;

   type_t *type;
} decl_t;

typedef struct scope_t {
   ht_t(char *, decl_t *) decls;
   type_t *ret;

   struct scope_t *parent;
} scope_t;

typedef struct checker_t {
   scope_t scope;
   scope_t *cur_scope;
   scope_t *file_scope;

   bool error;
   size_t errors;
} checker_t;

// Checker
bool checker_check(node_t *AST);
void checker_check_start(node_t *node, checker_t *ckr);
void checker_check_end(node_t *node, checker_t *ckr);

decl_t *checker_get_decl(checker_t *ckr, char *ident);
type_t *checker_reslove_type(checker_t *ckr, type_t *type);
type_t *checker_reslove_base_type(checker_t *ckr, type_t *type);

void checker_set_decl(checker_t *ckr, char *ident, decl_t decl);
bool checker_decl_exists_cur(checker_t *ckr, char *ident);
void free_decl(void *decl);

// Check
bool checker_check_check(checker_t *ckr, node_t *node);

#define NODE(ident, ...) bool checker_check_##ident(checker_t *ckr, node_t *n);
   NODE_TYPES
#undef NODE

bool checker_check_BLOCK_end(checker_t *ckr, node_t *block);
bool checker_check_FUNCTION_DECLARATION_end(checker_t *ckr, node_t *n);

// Infer
type_t *checker_infer_expression(checker_t *ckr, node_t *expr);
type_t *checker_infer_expression_no_res(checker_t *ckr, node_t *expr);
type_t *checker_infer_binexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_accessexpr(checker_t *ckr, node_t *expr);
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
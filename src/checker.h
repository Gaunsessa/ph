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
   ht_t(char *, type_t *) decls;
   type_t *ret;

   struct scope_t *parent;
} scope_t;

typedef struct checker_t {
   scope_t scope;
   scope_t *cur_scope;

   bool error;
   size_t errors;
} checker_t;

bool checker_check(node_t *AST);
void checker_check_start(node_t *node, checker_t *ckr);
void checker_check_end(node_t *node, checker_t *ckr);

type_t *checker_get_type(checker_t *ckr, char *ident);
type_t *checker_reslove_type(checker_t *ckr, type_t *type);

// Check
bool checker_check_check(checker_t *ckr, node_t *node);

bool checker_check_file(checker_t *ckr, node_t *file);

bool checker_check_block(checker_t *ckr, node_t *block);
bool checker_check_block_end(checker_t *ckr, node_t *block);

bool checker_check_func_decl(checker_t *ckr, node_t *funcd);
bool checker_check_func_decl_end(checker_t *ckr, node_t *funcd);

bool checker_check_binexpr(checker_t *ckr, node_t *expr);
bool checker_check_uryexpr(checker_t *ckr, node_t *expr);
bool checker_check_callexpr(checker_t *ckr, node_t *expr);
bool checker_check_identifier(checker_t *ckr, node_t *ident);
bool checker_check_literal(checker_t *ckr, node_t *lit);

bool checker_check_data_type(checker_t *ckr, node_t *dtype);
bool checker_check_var_decl(checker_t *ckr, node_t *vard);
bool checker_check_if(checker_t *ckr, node_t *stmt);
bool checker_check_for(checker_t *ckr, node_t *stmt);
bool checker_check_break(checker_t *ckr, node_t *brk);
bool checker_check_continue(checker_t *ckr, node_t *cnt);
bool checker_check_return(checker_t *ckr, node_t *retn);

// Infer
type_t *checker_infer_expression(checker_t *ckr, node_t *expr);
type_t *checker_infer_binexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_uryexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_callexpr(checker_t *ckr, node_t *expr);
type_t *checker_infer_callexpr_funct(checker_t *ckr, node_t *expr);
type_t *checker_infer_literal(checker_t *ckr, node_t *lit);
type_t *checker_infer_identifier(checker_t *ckr, node_t *ident);

type_t *checker_infer_var_decl(checker_t *ckr, node_t *vard);
type_t *checker_infer_alias(checker_t *ckr, node_t *alias);

#endif
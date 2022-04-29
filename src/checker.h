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

// NODE(NONE)                                                                                                                 \
// NODE(EMPTY)                                                                                                                \

// NODE(DATA_TYPE, type_t *, type)                                                                                            \

// NODE(BREAK)                                                                                                                \
// NODE(CONTINUE)                                                                                                             

type_t *checker_file(checker_t *ckr, node_t *file);

type_t *checker_statement(checker_t *ckr, node_t *stmt);

type_t *checker_block(checker_t *ckr, node_t *block);

type_t *checker_expression(checker_t *ckr, node_t *expr);
type_t *checker_binary_expression(checker_t *ckr, node_t *expr);
type_t *checker_unary_expression(checker_t *ckr, node_t *expr);
type_t *checker_call_expression(checker_t *ckr, node_t *expr);

type_t *checker_literal(checker_t *ckr, node_t *lit);

type_t *checker_identifier(checker_t *ckr, node_t *ident);

type_t *checker_variable_declaration(checker_t *ckr, node_t *vard);
type_t *checker_function_declaration(checker_t *ckr, node_t *funcd);

type_t *checker_if(checker_t *ckr, node_t *ifstmt);
type_t *checker_for(checker_t *ckr, node_t *forstmt);

type_t *checker_return(checker_t *ckr, node_t *ret);

type_t *checker_alias(checker_t *ckr, node_t *alias);

// type_t *checker_reslove_type(checker_t *ckr, type_t *type);
bool checker_can_reslove_type(checker_t *ckr, type_t *type);
type_t *checker_reslove_type(checker_t *ckr, type_t *type);
type_t *checker_get_type(checker_t *ckr, char *ident);

// type_t *checker_variable_declartion_type(checker_t *ckr, node_t *vard);

// type_t *checker_infer_stmt_type(checker_t *ckr, node_t *stmt);

#endif
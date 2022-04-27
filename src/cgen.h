#ifndef CGEN_H
#define CGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <print.h>
#include <ht.h>

#include "util.h"
#include "node.h"
#include "lexer.h"

typedef dynarr_t(char) buf_t;

char *cgen_generate(node_t *AST);

void cgen_file(buf_t buf, node_t *file);

void cgen_statement(buf_t buf, node_t *stmt);

void cgen_expression(buf_t buf, node_t *expr);

void cgen_if(buf_t buf, node_t *ifstmt);

void cgen_for(buf_t buf, node_t *forstmt);

void cgen_break(buf_t buf, node_t *brk);

void cgen_continue(buf_t buf, node_t *cnt);

void cgen_return(buf_t buf, node_t *ret);

void cgen_variable_declaration(buf_t buf, node_t *vard);

void cgen_block(buf_t buf, node_t *block);

void cgen_type(buf_t buf, node_t *type);

void cgen_identifier(buf_t buf, node_t *ident);

void cgen_literal(buf_t buf, node_t *lit);

#endif
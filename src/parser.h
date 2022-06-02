#ifndef PARSER_H
#define PARSER_H

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

typedef struct parser_t {
   lexer_t *lexer;
   token_t lookaheads[4];

   wchar_t *module;
   type_handler_t *ty_hdl;

   bool req_semi;
} parser_t;

node_t *parser_parse(size_t amt, FILE **files, type_handler_t *ty_hdl);

node_t *parser_parse_file(FILE *f, type_handler_t *ty_hdl);

node_t *parser_file(parser_t *p);

node_t *parser_statement(parser_t *p);
node_t *parser_parenthesized_statement(parser_t *p);

node_t *parser_expression(parser_t *p);
node_t *_parser_binary_expression(parser_t *p, int precedence);
node_t *_parser_expression(parser_t *p, int precedence);

node_t *parser_sign_expression(parser_t *p);
node_t *parser_pre_incdec_expression(parser_t *p);
node_t *parser_struct_literal(parser_t *p);
node_t *parser_feild_expression(parser_t *p);
node_t *parser_method_expression(parser_t *p);
node_t *parser_post_incdec_expression(parser_t *p);
node_t *parser_not_expression(parser_t *p);
node_t *parser_subscript_expression(parser_t *p);
node_t *parser_deref_expression(parser_t *p);
node_t *parser_addr_expression(parser_t *p);
node_t *parser_cast_expression(parser_t *p);
node_t *parser_call_expression(parser_t *p);
node_t *parser_path_expression(parser_t *p);

node_t *parser_primary_expression(parser_t *p);
node_t *parser_module_feild_expression(parser_t *p);

node_t *parser_if(parser_t *p);

node_t *parser_for(parser_t *p);
node_t *parser_break(parser_t *p);
node_t *parser_continue(parser_t *p);
node_t *parser_defer(parser_t *p);

node_t *parser_arrow_block(parser_t *p);

node_t *parser_type(parser_t *p);
type_idx _parser_type(parser_t *p);
type_idx parser_base_type(parser_t *p);
type_idx parser_ptr_type(parser_t *p);
type_idx parser_array_type(parser_t *p);
type_idx parser_function_type(parser_t *p);
type_idx parser_struct_type(parser_t *p);

node_t *parser_struct(parser_t *p);

node_t *parser_impl(parser_t *p);

node_t *parser_uninit(parser_t *p);

node_t *parser_literal(parser_t *p);

node_t *parser_identifer(parser_t *p);
wchar_t *parser_identifer_str(parser_t *p);

node_t *parser_variable_declaration(parser_t *p);
node_t *parser_multi_variable_declaration(parser_t *p);

node_t *parser_function_declaration(parser_t *p);

node_t *parser_alias(parser_t *p);

node_t *parser_return(parser_t *p);

node_t *parser_block(parser_t *p);

void parser_skip_newlines(parser_t *p);

dynarr_t(node_t *) parser_sep_list(parser_t *p, TOKEN_TYPE sep, TOKEN_TYPE end);
dynarr_t(node_t *) parser_sep_list_func(parser_t *p, TOKEN_TYPE sep, TOKEN_TYPE end, node_t *(*v_func)(parser_t *p), node_t *(*u_func)(parser_t *p));

#define parser_eat(p, ...) _parser_eat(p, sizeof((TOKEN_TYPE[]) {__VA_ARGS__}) / sizeof(TOKEN_TYPE), __VA_ARGS__)
token_t _parser_eat(parser_t *p, size_t n, ...);

#endif

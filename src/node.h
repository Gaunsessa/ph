#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <dynarr.h>

#include "util.h"
#include "lexer.h"
#include "types.h"

#define NODE_TYPES                                                                                                               \
      NODE(NONE)                                                                                                                 \
      NODE(EMPTY)                                                                                                                \
      NODE(MULTI, dynarr_t(struct node_t *), nodes)                                                                              \
      NODE(FILE, dynarr_t(struct node_t *), stmts)                                                                               \
      NODE(BLOCK, dynarr_t(struct node_t *), stmts)                                                                              \
      NODE(BINARY_EXPRESSION, TOKEN_TYPE, op, struct node_t *, left, struct node_t *, right)                                     \
      NODE(CALL_EXPRESSION, struct node_t *, func, dynarr_t(struct node_t *), args, bool, curried)                               \
      NODE(SUBSCRIPT_EXPRESSION, struct node_t *, ident, struct node_t *, expr)                                                  \
      NODE(SIGN_EXPRESSION, TOKEN_TYPE, op, struct node_t *, expr)                                                               \
      NODE(INCDEC_EXPRESSION, TOKEN_TYPE, op, struct node_t *, expr, bool, prefix)                                               \
      NODE(NOT_EXPRESSION, TOKEN_TYPE, op, struct node_t *, expr)                                                                \
      NODE(DEREF_EXPRESSION, struct node_t *, expr)                                                                              \
      NODE(ADDR_EXPRESSION, struct node_t *, expr)                                                                               \
      NODE(CAST_EXPRESSION, struct node_t *, type, struct node_t *, expr)                                                        \
      NODE(ACCESS_EXPRESSION, struct node_t *, expr, char *, member, bool, ptr)                                                  \
      NODE(ALIAS, struct node_t *, type)                                                                                         \
      NODE(STRUCT, struct node_t *, type)                                                                                        \
      NODE(IDENTIFIER, char *, value)                                                                                            \
      NODE(NUMBER_LITERAL, size_t, value)                                                                                        \
      NODE(FLOAT_LITERAL, size_t, integer, size_t, fraction)                                                                     \
      NODE(STRING_LITERAL, char *, span)                                                                                         \
      NODE(DATA_TYPE, type_t *, type)                                                                                            \
      NODE(VARIABLE_DECLARATION, struct node_t *, ident, struct node_t *, type, struct node_t *, expr, bool, immutable)          \
      NODE(FUNCTION_DECLARATION, struct node_t *, type, struct node_t *, stmt)                                                   \
      NODE(UNINIT)                                                                                                               \
      NODE(IF, struct node_t *, cond, struct node_t *, truecase, struct node_t *, falsecase)                                     \
      NODE(FOR, struct node_t *, init, struct node_t *, cond, struct node_t *, post, struct node_t *, stmt)                      \
      NODE(BREAK)                                                                                                                \
      NODE(CONTINUE)                                                                                                             \
      NODE(RETURN, struct node_t *, value)                                                                                       \

typedef enum NODE_TYPE {
   _NODE_ = -2,
#define NODE(ident, ...) NODE_##ident,
   NODE_TYPES
#undef NODE
} NODE_TYPE;

typedef struct node_t {
   NODE_TYPE type;

   union {
#define NODE(ident, ...) struct { M_VASEMI(M_CAT2ARGS(__VA_ARGS__)) } ident;
      NODE_TYPES
#undef NODE
   };
} node_t;

#define node_init(...) ({ node_t *n = calloc(1, sizeof(node_t)); memcpy(n, &(node_t) { __VA_ARGS__ }, sizeof(node_t)); n; })

void node_walker(node_t *node, void (*start)(node_t *node), void (*end)(node_t *node));

void node_free(node_t *node);

void print_node(node_t *node);

#endif

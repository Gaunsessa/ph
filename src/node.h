#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include <dynarr.h>

#include "util.h"
#include "lexer.h"
#include "types.h"
#include "symbol.h"

#define NODE_TYPES                             \
      /* Others */                             \
      NODE(NONE)                               \
      NODE(EMPTY)                              \
      NODE(MULTI,                              \
         dynarr_t(struct node_t *), nodes      \
      )                                        \
      NODE(SOURCE,                             \
         dynarr_t(struct node_t *), funcs,     \
         dynarr_t(struct node_t *), vars       \
      )                                        \
      NODE(PROJECT,                            \
         dynarr_t(struct node_t *), modules    \
      )                                        \
      NODE(FILE,                               \
         wchar_t *, name,                      \
         dynarr_t(wchar_t *), imports,         \
         dynarr_t(struct node_t *), stmts      \
      /* Types */                              \
      )                                        \
      NODE(DATA_TYPE,                          \
         struct node_t *, type                 \
      )                                        \
      NODE(TYPE_IDX,                           \
         type_idx, type                        \
      )                                        \
      NODE(PTR_TYPE,                           \
         struct node_t *, base                 \
      )                                        \
      NODE(FUNCTION_TYPE,                      \
         wchar_t *, self,                      \
         dynarr_t(wchar_t *), arg_names,       \
         dynarr_t(struct node_t *), arg_types, \
         struct node_t *, ret                  \
      )                                        \
      /* Statements */                         \
      NODE(BLOCK,                              \
         dynarr_t(struct node_t *), stmts      \
      )                                        \
      NODE(VARIABLE_DECLARATION,               \
         struct node_t *, ident,               \
         struct node_t *, type,                \
         struct node_t *, expr,                \
         bool, immutable                       \
      )                                        \
      NODE(IF,                                 \
         struct node_t *, cond,                \
         struct node_t *, truecase,            \
         struct node_t *, falsecase            \
      )                                        \
      NODE(FOR,                                \
         struct node_t *, init,                \
         struct node_t *, cond,                \
         struct node_t *, post,                \
         struct node_t *, stmt,                \
         bool, dor                             \
      )                                        \
      NODE(BREAK)                              \
      NODE(CONTINUE)                           \
      NODE(RETURN,                             \
         struct node_t *, value                \
      )                                        \
      NODE(DEFER,                              \
         struct node_t *, expr                 \
      )                                        \
      NODE(IMPL,                               \
         struct node_t *, type,                \
         dynarr_t(struct node_t *), funcs      \
      )                                        \
      /* Expressions */                        \
      NODE(IDENTIFIER,                         \
         wchar_t *, value                      \
      )                                        \
      NODE(NUMBER_LITERAL,                     \
         size_t, value                         \
      )                                        \
      NODE(FLOAT_LITERAL,                      \
         size_t, integer,                      \
         size_t, fraction                      \
      )                                        \
      NODE(STRING_LITERAL,                     \
         wchar_t *, span                       \
      )                                        \
      NODE(STRUCT_LITERAL,                     \
         struct node_t *, type,                \
         dynarr_t(wchar_t *), idents,          \
         dynarr_t(struct node_t *), exprs      \
      )                                        \
      NODE(BINARY_EXPRESSION,                  \
         TOKEN_TYPE, op,                       \
         struct node_t *, left,                \
         struct node_t *, right                \
      )                                        \
      NODE(CALL_EXPRESSION,                    \
         struct node_t *, func,                \
         dynarr_t(struct node_t *), args,      \
         bool, curried                         \
      )                                        \
      NODE(PATH_EXPRESSION,                    \
         struct node_t *, module,              \
         struct node_t *, expr                 \
      )                                        \
      NODE(FEILD_EXPRESSION,                   \
         struct node_t *, expr,                \
         wchar_t *, member,                    \
         bool, ptr,                            \
         bool, module                          \
      )                                        \
      NODE(METHOD_EXPRESSION,                  \
         struct node_t *, expr,                \
         wchar_t *, member,                    \
         dynarr_t(struct node_t *), args,      \
         bool, curried,                        \
         bool, ptr,                            \
         bool, module                          \
      )                                        \
      NODE(SUBSCRIPT_EXPRESSION,               \
         struct node_t *, ident,               \
         struct node_t *, expr                 \
      )                                        \
      NODE(SIGN_EXPRESSION,                    \
         TOKEN_TYPE, op,                       \
         struct node_t *, expr                 \
      )                                        \
      NODE(INCDEC_EXPRESSION,                  \
         TOKEN_TYPE, op,                       \
         struct node_t *, expr,                \
         bool, prefix                          \
      )                                        \
      NODE(NOT_EXPRESSION,                     \
         TOKEN_TYPE, op,                       \
         struct node_t *, expr                 \
      )                                        \
      NODE(DEREF_EXPRESSION,                   \
         struct node_t *, expr                 \
      )                                        \
      NODE(ADDR_EXPRESSION,                    \
         struct node_t *, expr                 \
      )                                        \
      NODE(CAST_EXPRESSION,                    \
         struct node_t *, type,                \
         struct node_t *, expr                 \
      )                                        \
      NODE(FUNCTION_DECLARATION,               \
         struct node_t *, type,                \
         struct node_t *, stmt                 \
      )                                        \
      NODE(STRUCT,                             \
         struct node_t *, type                 \
      )                                        \
      NODE(ALIAS,                              \
         struct node_t *, type                 \
      )                                        \
      NODE(UNINIT)                             \

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

void node_walk(node_t *node, sym_table_t *tbl, size_t scope, bool (*special)(node_t *node), void (*start)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope), void (*end)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope));
void _node_walk(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope, bool (*special)(node_t *node), void (*start)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope), void (*end)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope));

void node_walker(node_t *node, bool (*special)(node_t *node), void (*start)(node_t *node), void (*end)(node_t *node));

void node_replace(node_t *onode, node_t nnode);

void node_free(node_t *node);

void print_node(node_t *node);

void print_node_type(NODE_TYPE type);

#endif

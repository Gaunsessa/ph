#ifndef PRIMS_H
#define PRIMS_H

#include <dynarr.h>
#include <stdio.h>

#include "node.h"
#include "types.h"

#define PRIM_TYPES                                                                                \
      PTYPE(S8,        signed char,               { printf("%d", *v); },   {})                    \
      PTYPE(U8,        unsigned char,             { printf("%i", *v); },   {})                    \
      PTYPE(S16,       signed short,              { printf("%hi", *v); },  {})                    \
      PTYPE(U16,       unsigned short,            { printf("%hu", *v); },  {})                    \
      PTYPE(S32,       signed int,                { printf("%d", *v); },   {})                    \
      PTYPE(U32,       unsigned int,              { printf("%u", *v); },   {})                    \
      PTYPE(S64,       signed long,               { printf("%ld", *v); },  {})                    \
      PTYPE(U64,       unsigned long,             { printf("%lu", *v); },  {})                    \
      PTYPE(S128,      signed long long,          { printf("%lld", *v); }, {})                    \
      PTYPE(U128,      unsigned long long,        { printf("%llu", *v); }, {})                    \
      PTYPE(F32,       float,                     { printf("%f", *v); },   {})                    \
      PTYPE(F64,       double,                    { printf("%f", *v); },   {})                    \
      PTYPE(F128,      long double,               { printf("%Lf", *v); },  {})                    \
      PTYPE(BOOL,      bool,                      { printf("%s", *v ? "true" : "false"); }, {}) \
      PTYPE(STR,       wchar_t *,                 { printf("%ls", *v); },   { free(*v); })        \
      PTYPE(DYNSTR,    dynarr_t(wchar_t *),       {                                               \
         for (int i = 0; i < dy_len(*v); i++)                                                     \
            printf("%ls ", dyi(*v)[i]);                                                           \
      }, {                                                                                        \
         for (int i = 0; i < dy_len(*v); i++)                                                     \
            free(dyi(*v)[i]);                                                                     \
         dy_free(*v);                                                                             \
      })                                                                                          \
      PTYPE(NODEPTR,   node_t *,                  { print_node(*v); },     { node_free(*v); })    \
      PTYPE(DYNNODE,   dynarr_t(struct node_t *), {                                               \
         for (int i = 0; i < dy_len(*v); i++)                                                     \
            _printtype(&dyi(*v)[i], PRIM_NODEPTR);                                                \
      }, {                                                                                        \
         for (int i = 0; i < dy_len(*v); i++) node_free(dyi(*v)[i]);                              \
         dy_free(*v);                                                                             \
      })                                                                                          \
      PTYPE(TYPEPTR, type_t *, { print_type(*v); }, {  })                                \

typedef enum PRIM_TYPE {
   PRIM_NONE,

#define PTYPE(ident, type, ...) PRIM_##ident,
   PRIM_TYPES
#undef PTYPE
} PRIM_TYPE;

static inline void _printtype(void *data, PRIM_TYPE type) {
   switch (type) {
      case PRIM_NONE:
         printf("None printable type!");
         break;

#define PTYPE(ident, type, pimpl, fimpl) case PRIM_##ident: { type *v = data; pimpl } break;
      PRIM_TYPES
#undef PTYPE
   }
}

static inline void _freetype(void *data, PRIM_TYPE type) {
   switch (type) {
      case PRIM_NONE:
         printf("None freeable type!");
         break;

#define PTYPE(ident, type, pimpl, fimpl) case PRIM_##ident: { type *v = data; fimpl } break;
      PRIM_TYPES
#undef PTYPE
   }
}

// This cant be undeffed and has to be at the end.
#define PTYPE(ident, type, ...) type: PRIM_##ident,
#define primtype(x) _Generic((x), PRIM_TYPES default: PRIM_NONE)

#endif
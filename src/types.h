#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <ht.h>
#include <dynarr.h>
#include <print.h>

#include "util.h"

#define BASE_TYPES                  \
      TYPE(NONE, "", "")            \
      TYPE(U8, "u8", "uint8_t")     \
      TYPE(U16, "u16", "uint16_t")  \
      TYPE(U32, "u32", "uint32_t")  \
      TYPE(U64, "u64", "uint64_t")  \
      TYPE(I8, "i8", "int8_t")      \
      TYPE(I16, "i16", "int16_t")   \
      TYPE(I32, "i32", "int32_t")   \
      TYPE(I64, "i64", "int64_t")   \
      TYPE(F32, "f32", "float")     \
      TYPE(F64, "f64", "double")    \
      TYPE(BOOL, "bool", "bool")    \
      TYPE(INT, "int", "int")       \
      TYPE(VOID, "void", "void")    \
      TYPE(STRING, "str", "string") \
      TYPE(FUNCTION, "", "")        \
      TYPE(TYPE_, "Type", "type")   \

typedef enum BASE_TYPE {
#define TYPE(ident, ...) BASE_##ident,
   BASE_TYPES
#undef TYPE
} BASE_TYPE;

typedef enum TYPE_TYPE {
   TYPE_NONE,
   TYPE_INFER,
   TYPE_BASE,
   TYPE_ALIAS,
   TYPE_TYPE_REF,
   TYPE_UNTYPED,
   TYPE_PTR,
   TYPE_ARRAY,
   TYPE_FUNCTION,
   TYPE_STRUCT,
} TYPE_TYPE;

typedef struct type_t {
   TYPE_TYPE type;
   char *name;

   bool distinct;

   union {
      // Base
      struct {
         BASE_TYPE base;
      };

      // TypeRef
      struct {
         struct type_t *ref;
      };

      // Untyped
      struct {
         struct type_t *uninfer;
      };

      // Function
      struct {
         dynarr_t(struct { char *name; struct type_t *type; }) args;
         struct type_t *ret;
      };

      // Struct
      struct {
         dynarr_t(struct { char *name; struct type_t *type; }) feilds;
      };

      // Ptr
      struct {
         struct type_t *ptr_base;
      };

      // Array
      struct {
         struct type_t *arr_base;
         size_t length;
      };

      // TODO: string, tuple, enums, maps, slices, dynarr, union, any
   };
} type_t;

type_t *BASE_UNTYPED_INT;
type_t *BASE_UNTYPED_FLOAT;

ht_t(BASE_TYPE, type_t *) BASE_TYPE_ENUM_VALUES;
ht_t(char *, type_t *) BASE_TYPE_STR_VALUES;

dynarr_t(type_t *) ALLOCATED_TYPES;

#define _type_init(...) ({ type_t *t = calloc(1, sizeof(type_t)); memcpy(t, &(type_t) { __VA_ARGS__ }, sizeof(type_t)); t; })

void type_module_init();

type_t *type_init(type_t type);
void type_free_all();

bool type_is_base(type_t *t);
bool type_is_numeric(type_t *t);
bool type_is_integer(type_t *t);
bool type_is_float(type_t *t);
bool type_is_ptr(type_t *t);
bool type_is_indexable(type_t *t);

type_t *type_deref_ref(type_t *t);

bool type_cmp(type_t *t1, type_t *t2);

const char *type_base_cname(type_t *t);

void print_type(type_t *type);

#endif
#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include <ht.h>
#include <dynarr.h>
#include <print.h>

#include "util.h"
#include "lexer.h"

/*
   -- Types --

   Base Types:
      Integers:
         u8 u16 u32 u64
         i8 i16 i32 i64
         int
         untyped-int

      Decimal:
         f32 f64
         untyped-float
      
      bool
      str
      void

   Compound Types:
      ptr
      function
      struct

   Rules:
      - Compare: Types in same type group can compare
      - Arithmetic: Types must be same to add
      - Cast: All numeric types can cast including ptrs

*/

#define BASE_TYPES                    \
      TYPE(NONE, L"", L"")            \
      TYPE(U8, L"u8", L"uint8_t")     \
      TYPE(U16, L"u16", L"uint16_t")  \
      TYPE(U32, L"u32", L"uint32_t")  \
      TYPE(U64, L"u64", L"uint64_t")  \
      TYPE(I8, L"i8", L"int8_t")      \
      TYPE(I16, L"i16", L"int16_t")   \
      TYPE(I32, L"i32", L"int32_t")   \
      TYPE(I64, L"i64", L"int64_t")   \
      TYPE(F32, L"f32", L"float")     \
      TYPE(F64, L"f64", L"double")    \
      TYPE(BOOL, L"bool", L"bool")    \
      TYPE(INT, L"int", L"int")       \
      TYPE(VOID, L"void", L"void")    \
      TYPE(STRING, L"str", L"str_t")  \
      TYPE(RUNE, L"rune", L"wchar_t") \
      // TYPE(FUNCTION, L"", L"")        \
      // TYPE(TYPE_, L"Type", L"type")   

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
   TYPE_UNTYPED,
   TYPE_PTR,
   TYPE_ARRAY,
   TYPE_FUNCTION,
   TYPE_STRUCT,
} TYPE_TYPE;

typedef struct type_t {
   TYPE_TYPE type;
   bool distinct;

   union {
      // Base
      struct {
         BASE_TYPE base;
      };

      // Alias
      struct {
         wchar_t *name;
      };

      // Untyped
      struct {
         struct type_t *uninfer;
      };

      // Function
      struct {
         struct { wchar_t *name; struct type_t *type; } self;
         dynarr_t(struct { wchar_t *name; struct type_t *type; }) args;
         struct type_t *ret;
      };

      // Struct
      struct {
         dynarr_t(struct { wchar_t *name; struct type_t *type; }) feilds;
         dynarr_t(struct { wchar_t *name; struct type_t *type; }) funcs;
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

typedef struct type_handler_t {
   ht_t(char *, type_t *) types;
   dynarr_t(type_t *) allocs;
} type_handler_t;

type_handler_t *type_handler_new();

wchar_t *type_to_str(type_t *t);

type_t *BASE_UNTYPED_INT;
type_t *BASE_UNTYPED_FLOAT;

ht_t(BASE_TYPE, type_t *) BASE_TYPE_ENUM_VALUES;
ht_t(wchar_t *, type_t *) BASE_TYPE_STR_VALUES;

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

struct checker_t;
bool type_cmp(struct checker_t *ckr, type_t *t1, type_t *t2);

const wchar_t *type_base_cname(type_t *t);

void print_type(type_t *type);

#endif
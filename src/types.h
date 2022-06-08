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
   TYPE_MODULE,
   TYPE_BASE,
   TYPE_UNTYPED,
   TYPE_ALIAS,
   TYPE_PTR,
   TYPE_ARRAY,
   TYPE_FUNCTION,
   TYPE_STRUCT,
} TYPE_TYPE;

typedef int type_idx;

typedef struct type_t {
   TYPE_TYPE type;

   wchar_t *name;
   wchar_t *module;

   union {
      // Base
      struct {
         BASE_TYPE base;
      };

      // Alias
      struct {
         bool distinct;
      };

      // Untyped
      struct {
         type_idx uninfer;
      };

      // Function
      struct {
         struct { wchar_t *name; type_idx type; } self;
         dynarr_t(struct { wchar_t *name; type_idx type; }) args;
         type_idx ret;
      };

      // Struct
      struct {
         dynarr_t(struct { wchar_t *name; type_idx type; }) feilds;
         dynarr_t(struct { wchar_t *name; type_idx type; }) funcs;
      };

      // Ptr
      struct {
         type_idx ptr_base;
      };

      // Array
      struct {
         type_idx arr_base;
         size_t length;
      };

      // TODO: string, tuple, enums, maps, slices, dynarr, union, any
   };
} type_t;

typedef struct type_handler_t {
   dynarr_t(type_t *) allocs;
} type_handler_t;

type_handler_t *type_handler_new();

void type_handler_free(type_handler_t *hnd);

type_idx type_init(type_handler_t *hnd, type_t type);
type_t *type_get(type_handler_t *hnd, type_idx idx);

// struct module_t;
// bool type_cmp(struct module_t *mod, type_idx t1, type_idx t2);
bool type_cmp(type_handler_t *hnd, type_idx t1, type_idx t2);

#endif
#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <dynarr.h>
#include <print.h>

#define BASE_TYPES                 \
      TYPE(U8, "u8", "uint8_t")    \
      TYPE(U16, "u16", "uint16_t") \
      TYPE(U32, "u32", "uint32_t") \
      TYPE(U64, "u64", "uint64_t") \
      TYPE(I8, "i8", "int8_t")     \
      TYPE(I16, "i16", "int16_t")  \
      TYPE(I32, "i32", "int32_t")  \
      TYPE(I64, "i64", "int64_t")  \
      TYPE(F32, "f32", "float")    \
      TYPE(F64, "f64", "double")   \
      TYPE(BOOL, "bool", "bool")   \
      TYPE(INT, "int", "int")      \
      TYPE(VOID, "void", "void")   \
      TYPE(FUNCTION, NULL, NULL)   \

typedef enum BASE_TYPE {
#define TYPE(ident, ...) BASE_##ident,
   BASE_TYPES
#undef TYPE
} BASE_TYPE;

typedef enum TYPE_TYPE {
   TYPE_NONE,
   TYPE_BASE,
   TYPE_PTR,
   TYPE_ARRAY,
   TYPE_FUNCTION,
   TYPE_STRUCT,
   TYPE_ENUM,
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

      // TODO: string, enums, maps, slices, dynarr, union, any
   };
} type_t;

#endif
#include "types.h"

type_handler_t *type_handler_new() {
   type_handler_t *handler = malloc(sizeof(type_handler_t));

   handler->allocs = dy_init(type_t *);

#define TYPE(ident, str, cstr) type_init(handler, (type_t) { TYPE_BASE, .base = BASE_##ident });
   BASE_TYPES
#undef TYPE

   return handler;
}

void type_handler_free(type_handler_t *hnd) {
   dy_free(hnd->allocs);

   free(hnd);
}

type_idx type_init(type_handler_t *hnd, type_t type) {
   type_t *ptype = malloc(sizeof(type));
   memcpy(ptype, &type, sizeof(type));

   dy_push(hnd->allocs, ptype);

   return dy_len(hnd->allocs) - 1;
}

type_t *type_get(type_handler_t *hnd, type_idx idx) {
   if (idx < 0) return NULL;

   return dyi(hnd->allocs)[idx];
}

bool type_cmp(type_handler_t *hnd, type_idx t1, type_idx t2) {
   if (t1 < 0 || t2 < 0) return false;

   type_t *type1 = type_get(hnd, t1);
   type_t *type2 = type_get(hnd, t2);

   if (type1->type != type2->type) return false;

   if (type1->name != NULL && type2->name != NULL && !wcscmp(type1->name, type2->name)) 
      return true;

   switch (type1->type) {
      case TYPE_BASE:
         return type1->base == type2->base;
      case TYPE_PTR:
         return type_cmp(hnd, type1->ptr_base, type2->ptr_base);
      case TYPE_ARRAY:
         return type1->length == type2->length && type_cmp(hnd, type1->arr_base, type2->arr_base); 
      case TYPE_FUNCTION:
         if (dy_len(type1->args) != dy_len(type2->args) || !type_cmp(hnd, type1->ret, type2->ret)) return false;

         for (int i = 0; i < dy_len(type1->args); i++)
            if (!type_cmp(hnd, dyi(type1->args)[i].type, dyi(type2->args)[i].type) ||
                wcscmp(dyi(type1->args)[i].name, dyi(type2->args)[i].name) != 0)
               return false;

         return true;
      case TYPE_STRUCT:
         if (dy_len(type1->feilds) != dy_len(type2->feilds)) return false;

         for (int i = 0; i < dy_len(type1->feilds); i++) {
            if (!type_cmp(hnd, dyi(type1->feilds)[i].type, dyi(type2->feilds)[i].type))
               return false;

            // if (wcscmp(dyi(type1->feilds)[i].name, dyi(type2->feilds)[i].name) != 0)
            //    return false;
         }

         return true;
      default: eprint("Unreachable Statement!", type1->type);
   }
}

bool type_is_numeric(type_t* type) {
   return type_is_integer(type) || type_is_float(type);
}

bool type_is_float(type_t *type) {
   return type->type == TYPE_BASE && M_COMPARE(type->base, BASE_F32, BASE_F64);
}

bool type_is_integer(type_t *type) {
   return type->type == TYPE_BASE && M_COMPARE(
      type->base, 
      BASE_INT, 
      BASE_I8, BASE_I16, BASE_I32, BASE_I64, 
      BASE_U8, BASE_U16, BASE_U32, BASE_U64
   );
}

bool type_is_ptr(type_t *type) {
   return type->type == TYPE_PTR;
}
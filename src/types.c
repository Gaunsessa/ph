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

// True = equal, False = not equal
// bool type_cmp(module_t *mod, type_idx t1, type_idx t2) {
   // ERROR("UNIMPLMENTED!");

   // if (t1 == NULL || t2 == NULL) return false;

   // // if (t1->type == TYPE_ALIAS) t1 = checker_resolve_type(mod, t1);
   // // if (t2->type == TYPE_ALIAS) t2 = checker_resolve_type(mod, t2);

   // if (t1 == NULL || t2 == NULL) return false;

   // if (t1->type == TYPE_UNTYPED || t2->type == TYPE_UNTYPED) {
   //    // if ((type_is_integer(t1) && type_is_integer(t2)) || ((type_is_float(t1) && type_is_float(t2))))
   //    return true;
   //    // else return false;
   // }

   // if (t1->type == TYPE_TYPE_REF) t1 = type_deref_ref(t1);
   // if (t2->type == TYPE_TYPE_REF) t2 = type_deref_ref(t2);

   // if (!(t1->type == TYPE_ALIAS || t2->type == TYPE_ALIAS) && t1->type != t2->type) 
   //    return false;

   // if (t1->distinct || t2->distinct) {
   //    if (t1->name == NULL || t2->name == NULL) 
   //       return false;

   //    return !wcscmp(t1->name, t2->name);
   // }

   // switch (t1->type) {
   //    case TYPE_BASE:
   //       return t1->base == t2->base;
   //    case TYPE_PTR:
   //       return type_cmp(mod, t1->ptr_base, t2->ptr_base);
   //    case TYPE_ARRAY:
   //       return t1->length == t2->length ? type_cmp(mod, t1->arr_base, t2->arr_base) : false; 
   //    case TYPE_FUNCTION:
   //       if (dy_len(t1->args) != dy_len(t2->args) || !type_cmp(mod, t1->ret, t2->ret)) return false;

   //       for (int i = 0; i < dy_len(t1->args); i++) {
   //          if (!type_cmp(mod, dyi(t1->args)[i].type, dyi(t2->args)[i].type))
   //             return false;

   //          if (wcscmp(dyi(t1->args)[i].name, dyi(t2->args)[i].name) != 0)
   //             return false;
   //       }

   //       return true;
   //    case TYPE_STRUCT:
   //       if (dy_len(t1->feilds) != dy_len(t2->feilds)) return false;

   //       for (int i = 0; i < dy_len(t1->feilds); i++) {
   //          if (!type_cmp(mod, dyi(t1->feilds)[i].type, dyi(t2->feilds)[i].type))
   //             return false;

   //          if (wcscmp(dyi(t1->feilds)[i].name, dyi(t2->feilds)[i].name) != 0)
   //             return false;
   //       }

   //       return true;
   //    default: eprint("Unreachable Statement!", t1->type);
   // }
// }

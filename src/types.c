#include "types.h"

void type_module_init() {
   ALLOCATED_TYPES       = dy_init(type_t *);
   BASE_TYPE_ENUM_VALUES = ht_init(BASE_TYPE, type_t *);
   BASE_TYPE_STR_VALUES  = ht_init_sv(type_t *);

#define TYPE(ident, str, cstr) ht_set(BASE_TYPE_ENUM_VALUES, BASE_##ident, _type_init(TYPE_BASE, .base = BASE_##ident, .name = cstr));
   BASE_TYPES
#undef TYPE

#define TYPE(ident, str, cstr) ht_set_sv(BASE_TYPE_STR_VALUES, str, _type_init(TYPE_BASE, .base = BASE_##ident, .name = cstr));
   BASE_TYPES
#undef TYPE

   BASE_UNTYPED_INT = type_init((type_t) { TYPE_UNTYPED, .uninfer = ht_get(BASE_TYPE_ENUM_VALUES, BASE_INT) });
   BASE_UNTYPED_FLOAT = type_init((type_t) { TYPE_UNTYPED, .uninfer = ht_get(BASE_TYPE_ENUM_VALUES, BASE_F32) });
}

type_t *type_init(type_t type) {
   type_t *ret = calloc(1, sizeof(type_t));
   memcpy(ret, &type, sizeof(type));

   dy_push(ALLOCATED_TYPES, ret);

   return ret;
}

void type_free_all() {
   while (dy_len(ALLOCATED_TYPES))
      free(dy_pop(ALLOCATED_TYPES));
}

bool type_is_base(type_t *t) {
   return t->type == TYPE_BASE;
}

bool type_is_numeric(type_t *t) {
   return (t->type == TYPE_BASE && M_COMPARE(
      t->base, BASE_U8,  BASE_U16, BASE_U32, BASE_U64,
               BASE_I8,  BASE_I32, BASE_I32, BASE_U64,
               BASE_F32, BASE_F64, BASE_INT
   )) || (t->type == TYPE_UNTYPED && type_is_numeric(t->uninfer));
}

bool type_is_integer(type_t *t) {
   return (t->type == TYPE_BASE && M_COMPARE(
      t->base, BASE_U8,  BASE_U16, BASE_U32, BASE_U64,
               BASE_I8,  BASE_I32, BASE_I32, BASE_U64,
               BASE_INT
   )) || (t->type == TYPE_UNTYPED && type_is_integer(t->uninfer));
}

bool type_is_float(type_t *t) {
   return (t->type == TYPE_BASE && M_COMPARE(t->base, BASE_F32, BASE_F64)) || 
   (t->type == TYPE_UNTYPED && type_is_float(t->uninfer));
}

bool type_is_ptr(type_t *t) {
   return t->type == TYPE_PTR;
}

bool type_is_indexable(type_t *t) {
   return t->type == TYPE_ARRAY;
}

type_t *type_deref_ref(type_t *t) {
   return t != NULL && t->type == TYPE_TYPE_REF ? type_deref_ref(t->ref) : t;
}

// True = equal, False = not equal
bool type_cmp(type_t *t1, type_t *t2) {
   if (t1 == NULL || t2 == NULL) return false;

   if (t1->type == TYPE_UNTYPED || t2->type == TYPE_UNTYPED) {
      if ((type_is_integer(t1) && type_is_integer(t2)) || ((type_is_float(t1) && type_is_float(t2))))
         return true;
      else return false;
   }

   if (t1->type == TYPE_TYPE_REF) t1 = type_deref_ref(t1);
   if (t2->type == TYPE_TYPE_REF) t2 = type_deref_ref(t2);

   if (!(t1->type == TYPE_ALIAS || t2->type == TYPE_ALIAS) && t1->type != t2->type) 
      return false;

   if (t1->distinct || t2->distinct) {
      if (t1->name == NULL || t2->name == NULL) 
         return false;

      return !strcmp(t1->name, t2->name);
   }

   switch (t1->type) {
      case TYPE_BASE:
         return t1->base == t2->base;
      case TYPE_PTR:
         return type_cmp(t1->ptr_base, t2->ptr_base);
      case TYPE_ARRAY:
         return t1->length == t2->length ? type_cmp(t1->arr_base, t2->arr_base) : false; 
      case TYPE_FUNCTION:
         if (dy_len(t1->args) != dy_len(t2->args) || !type_cmp(t1->ret, t2->ret)) return false;

         for (int i = 0; i < dy_len(t1->args); i++) {
            if (!type_cmp(dyi(t1->args)[i].type, dyi(t2->args)[i].type))
               return false;

            if (strcmp(dyi(t1->args)[i].name, dyi(t2->args)[i].name) != 0)
               return false;
         }

         return true;
      case TYPE_STRUCT:
         if (dy_len(t1->feilds) != dy_len(t2->feilds)) return false;

         for (int i = 0; i < dy_len(t1->feilds); i++) {
            if (!type_cmp(dyi(t1->feilds)[i].type, dyi(t2->feilds)[i].type))
               return false;

            if (strcmp(dyi(t1->feilds)[i].name, dyi(t2->feilds)[i].name) != 0)
               return false;
         }

         return true;
      default: eprint("Unreachable Statement!", t1->type);
   }
}

void print_type(type_t *type) {
   switch (type->type) {
      case TYPE_NONE:
         printf("Type: None | %s\n", type->name);
         break;
      case TYPE_BASE:
         printf("Type: Base | %s\n", type->name);
         break;
      case TYPE_PTR:
         printf("Type: Ptr | ");
         print_type(type->ptr_base);
         break;
      case TYPE_FUNCTION:
         printf("Type: Function | ");
         for (int i = 0; i < dy_len(type->args); i++) {
            printf("%s: ", dyi(type->args)[i].name);

            print_type(dyi(type->args)[i].type);

            printf(" ");
         }
         break;
      default: printf("Not printable type!\n");
   }
}
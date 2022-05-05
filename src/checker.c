#include "checker.h"

void _check_start(node_t *node) {
   checker_check_start(node, NULL);
}

void _check_end(node_t *node) {
   checker_check_end(node, NULL);
}

bool checker_check(node_t *AST) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->scope = (scope_t) {
      ht_init_sv(decl_t *),
      NULL,
      NULL,
   };

   ckr->cur_scope = &ckr->scope;
   ckr->file_scope = &ckr->scope;
   ckr->error = false;

   checker_check_start(AST, ckr);
   checker_check_end(AST, ckr);

   node_walker(AST, _check_start, _check_end);

   return ckr->errors == 0;
}

void checker_check_start(node_t *node, checker_t *ckr) {
   static checker_t *check = NULL;
   if (ckr != NULL) check = ckr;
   else {
      switch (node->type) {
#define NODE(ident, ...) case NODE_##ident: checker_check_##ident(check, node); return;
         NODE_TYPES
#undef NODE
         default: return;
      }
   }

}

void checker_check_end(node_t *node, checker_t *ckr) {
   static checker_t *check = NULL;
   if (ckr != NULL) check = ckr;
   else {
      switch (node->type) {
         case NODE_BLOCK: checker_check_BLOCK_end(check, node); return;
         case NODE_FUNCTION_DECLARATION: checker_check_FUNCTION_DECLARATION_end(check, node); return;
         default: return;
      }
   }
}

type_t *checker_reslove_type(checker_t *ckr, type_t *type) {
   if (type == NULL) return NULL;

   if (type->type == TYPE_ALIAS) {
      decl_t *dec = checker_get_decl(ckr, type->name);

      if (dec == NULL || dec->type == NULL) return NULL;
      else return checker_reslove_type(ckr, dec->type);
   }

   return type;
}

type_t *checker_reslove_base_type(checker_t *ckr, type_t *type) {
   if (type == NULL) return NULL;

   if (type->type == TYPE_ALIAS) {
      type = checker_reslove_type(ckr, type);
      if (type == NULL) return NULL;
   }

   switch (type->type) {
      case TYPE_PTR: return checker_reslove_base_type(ckr, type->ptr_base);
      case TYPE_ARRAY: return checker_reslove_base_type(ckr, type->arr_base);
      case TYPE_UNTYPED: return checker_reslove_base_type(ckr, type->uninfer);

      default: return type;
   }
}

decl_t *checker_get_decl(checker_t *ckr, char *ident) {
   scope_t *cur = ckr->cur_scope;

   while (cur != NULL) {
      if (ht_exists_sv(cur->decls, ident))
         return ht_get_sv(cur->decls, ident);

      cur = cur->parent;
   }

   return NULL;
}

void checker_set_decl(checker_t *ckr, char *ident, decl_t decl) {
   decl_t *dec = malloc(sizeof(decl_t));
   memcpy(dec, &decl, sizeof(decl_t));

   ht_set_sv(ckr->cur_scope->decls, ident, dec);
}

bool checker_decl_exists_cur(checker_t *ckr, char *ident) {
   return ht_exists_sv(ckr->cur_scope->decls, ident);
}

void free_decl(void *decl) {
   free(*(decl_t **)decl);
}
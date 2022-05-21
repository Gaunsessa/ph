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
      ht_init_sv(wchar_t, type_t *),
      ht_init_sv(wchar_t, type_t *),
      NULL,
      NULL,
   };

   ckr->cur_scope = &ckr->scope;
   ckr->file_scope = NULL;
   ckr->errors = 0;
   ckr->error = false;

   // TODO:
   //       change checker_t to support multiple scopes at the same time
   //       run checker_check_FILE on every file before walking them
   //       implment infering from cross file/scopes

   checker_check_start(AST, ckr);
   checker_check_end(AST, ckr);

   node_walker(AST, checker_check_special, _check_start, _check_end);

   return ckr->errors == 0;
}

bool checker_check_special(node_t *node) {
   switch (node->type) {
      case NODE_VARIABLE_DECLARATION:
         node_walker(node->VARIABLE_DECLARATION.expr, checker_check_special, _check_start, _check_end);
         node_walker(node->VARIABLE_DECLARATION.type, checker_check_special, _check_start, _check_end);

         return true;
      case NODE_FUNCTION_DECLARATION:
         node_walker(node->FUNCTION_DECLARATION.stmt, checker_check_special, _check_start, _check_end);

         return true;
      case NODE_FOR:
         node_walker(node->FOR.stmt, checker_check_special, _check_start, _check_end);
         node_walker(node->FOR.post, checker_check_special, _check_start, _check_end);

         return true;
      case NODE_STRUCT:

         return true;
      default: return false;
   }
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
         case NODE_FILE:
            check->file_scope = NULL;
         case NODE_BLOCK:
         case NODE_IMPL:
         // case NODE_ACCESS_EXPRESSION:
         case NODE_FUNCTION_DECLARATION: checker_pop_scope(check);
         default: return;
      }
   }
}

bool checker_check_node(checker_t *ckr, node_t *node) {
   switch (node->type) {
#define NODE(ident, ...) case NODE_##ident: return checker_check_##ident(ckr, node);
      NODE_TYPES
#undef NODE
      default: return true;
   }
}

void checker_push_scope(checker_t *ckr) {
   scope_t *scope = malloc(sizeof(scope_t));

   scope->decls = ht_init_sv(wchar_t, type_t *);
   scope->types = ht_init_sv(wchar_t, type_t *);
   scope->parent = ckr->cur_scope;
   scope->ret = ckr->cur_scope->ret;

   ckr->cur_scope = scope;
}

void checker_pop_scope(checker_t *ckr) {
   scope_t *scope = ckr->cur_scope;

   ht_free(scope->decls);
   ht_free(scope->types);
   ckr->cur_scope = scope->parent;

   free(scope);
}

// type_t *checker_reslove_typedef(checker_t *ckr, type_t *type) {
//    if (type == NULL) return NULL;

//    if (type->type == TYPE_ALIAS) {
//       type_t *t = checker_get_decl(ckr, type->name,);

//       if (t == NULL) return NULL;
//       else return checker_reslove_typedef(ckr, dec->type);
//    }

//    return type;
// }

type_t *checker_reslove_type(checker_t *ckr, type_t *type) {
   if (type == NULL) return NULL;

   if (type->type == TYPE_ALIAS) {
      type_t *t = checker_get_decl(ckr, type->name, true);

      if (t == NULL) return NULL;
      else return checker_reslove_type(ckr, t);
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

type_t *checker_get_decl(checker_t *ckr, wchar_t *ident, bool typedf) {
   scope_t *cur = ckr->cur_scope;


   while (cur != NULL) {
      typeof(cur->decls) scope = typedf ? (void *)cur->types : cur->decls;

      if (ht_exists_sv(scope, ident))
         return ht_get_sv(scope, ident);

      cur = cur->parent;
   }

   return NULL;
}

void checker_set_decl(checker_t *ckr, wchar_t *ident, type_t *type, bool typedf) {
   ht_set_sv(typedf ? (void *)ckr->cur_scope->types : ckr->cur_scope->decls, ident, type);
}

bool checker_decl_exists_cur(checker_t *ckr, wchar_t *ident) {
   return ht_exists_sv(ckr->cur_scope->decls, ident);
}

// void free_decl(void *decl) {
//    free(*(decl_t **)decl);
// }
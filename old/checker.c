#include "checker.h"

void _check_start(node_t *node) {
   checker_check_start(node, NULL, NULL);
}

void _check_end(node_t *node) {
   checker_check_end(node, NULL, NULL);
}

bool checker_check(node_t *AST, type_handler_t *type_handler) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->type_handler = type_handler;
   ckr->modules = ht_init_sv(wchar_t, module_t *);

   ckr->errors = 0;
   ckr->error = false;

   // TODO:
   //       change checker_t to support multiple scopes at the same time
   //       run checker_check_FILE on every file before walking them
   //       implment infering from cross file/scopes

   for (int i = 0; i < dy_len(AST->PROJECT.modules); i++) {
      wchar_t *name = dyi(AST->PROJECT.modules)[i]->FILE.name;

      if (!ht_exists_sv(ckr->modules, name)) {
         module_t *mod   = malloc(sizeof(module_t));
         mod->scope      = checker_scope_new();
         mod->cur_scope  = mod->scope;
         mod->file_scope = NULL;

         ht_set_sv(ckr->modules, name, mod);
      }

      checker_check_FILE(ckr, ht_get_sv(ckr->modules, name), dyi(AST->PROJECT.modules)[i]);
   }

   for (int i = 0; i < dy_len(AST->PROJECT.modules); i++) {
      module_t *mod = ht_get_sv(ckr->modules, dyi(AST->PROJECT.modules)[i]->FILE.name);

      checker_check_start(AST, ckr, mod);
      checker_check_end(AST, ckr, mod);

      for (int j = 0; j < dy_len(dyi(AST->PROJECT.modules)[i]->FILE.stmts); j++)
         node_walker(dyi(dyi(AST->PROJECT.modules)[i]->FILE.stmts)[j], checker_check_special, _check_start, _check_end);
   }

   return ckr->errors == 0;
}

bool checker_check_special(node_t *node) {
   switch (node->type) {
      case NODE_FILE:
         return true;
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
      case NODE_PATH_EXPRESSION:
         return true;
      case NODE_STRUCT:

         return true;
      default: return false;
   }
}

void checker_check_start(node_t *node, checker_t *ckr, module_t *mod) {
   static checker_t *check = NULL;
   static module_t *module = NULL;

   if (ckr != NULL && mod != NULL) { 
      check = ckr; 
      module = mod;
   } else {
      switch (node->type) {
#define NODE(ident, ...) case NODE_##ident: checker_check_##ident(check, module, node); return;
         NODE_TYPES
#undef NODE
         default: return;
      }
   }
}

void checker_check_end(node_t *node, checker_t *ckr, module_t *mod) {
   static checker_t *check = NULL;
   static module_t *module = NULL;

   if (ckr != NULL && mod != NULL) { 
      check = ckr; 
      module = mod;
   } else {
      switch (node->type) {
         // case NODE_FILE:
         //    module->file_scope = NULL;
         case NODE_BLOCK:
         case NODE_IMPL:
         // case NODE_ACCESS_EXPRESSION:
         case NODE_FUNCTION_DECLARATION: checker_pop_scope(module);
         default: return;
      }
   }
}

bool checker_check_node(checker_t *ckr, module_t *mod, node_t *node) {
   switch (node->type) {
#define NODE(ident, ...) case NODE_##ident: return checker_check_##ident(ckr, mod, node);
      NODE_TYPES
#undef NODE
      default: return true;
   }
}

scope_t *checker_scope_new() {
   scope_t *scope = calloc(1, sizeof(scope_t));

   scope->decls = ht_init_sv(wchar_t, type_idx);
   scope->types = ht_init_sv(wchar_t, type_idx);

   return scope;
}

void checker_scope_free(scope_t *scope) {
   ht_free(scope->decls);
   ht_free(scope->types);

   free(scope);
}

void checker_push_scope(module_t *mod) {
   scope_t *scope = checker_scope_new();

   scope->parent = mod->cur_scope;
   scope->ret = mod->cur_scope->ret;

   mod->cur_scope = scope;
}

void checker_pop_scope(module_t *mod) {
   scope_t *parent = mod->cur_scope->parent;
   
   checker_scope_free(mod->cur_scope);

   mod->cur_scope = parent;
}

type_idx checker_resolve_base_type(module_t *mod, type_idx idx) {
   if (idx < 0) return -1;

   type_t *type = type_get(NULL, idx);

   switch (type->type) {
      case TYPE_PTR: return checker_resolve_base_type(mod, type->ptr_base);
      case TYPE_ARRAY: return checker_resolve_base_type(mod, type->arr_base);
      case TYPE_UNTYPED: return checker_resolve_base_type(mod, type->uninfer);

      default: return idx;
   }
}

type_idx checker_get_decl(module_t *mod, wchar_t *ident, bool typedf) {
   scope_t *cur = mod->cur_scope;

   while (cur != NULL) {
      typeof(cur->decls) scope = typedf ? (void *)cur->types : cur->decls;

      if (ht_exists_sv(scope, ident))
         return ht_get_sv(scope, ident);

      cur = cur->parent;
   }

   return -1;
}

type_idx checker_get_decl_both(module_t *mod, wchar_t *ident) {
   type_idx type = checker_get_decl(mod, ident, false);
   if (type < 0) type = checker_get_decl(mod, ident, true);

   return type;
}

void checker_set_decl(module_t *mod, wchar_t *ident, type_idx idx, bool typedf) {
   ht_set_sv(typedf ? (void *)mod->cur_scope->types : mod->cur_scope->decls, ident, idx);
}

bool checker_decl_exists_cur(module_t *mod, wchar_t *ident) {
   return ht_exists_sv(mod->cur_scope->decls, ident);
}

// void free_decl(void *decl) {
//    free(*(decl_t **)decl);
// }
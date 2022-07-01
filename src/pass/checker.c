#include "checker.h"

#define error(...) { printf(__VA_ARGS__); printf("\n"); ckr->errors++; return; }

bool _checker_special(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope) { return checker_special(node, NULL, tbl, mod, scope, hscope); }
void _checker_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope)                   { return checker_start(node, NULL, tbl, mod, scope); }
void _checker_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope)                     { return checker_end(node, NULL, tbl, mod, scope); }

void checker_pass(node_t *AST, sym_table_t *tbl) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->scopes = dy_init(ckr_scope_t *);

   ckr->errors = 0;

   checker_special(NULL, ckr, NULL, NULL, 0, 0);
   checker_start(NULL, ckr, NULL, NULL, 0);
   checker_end(NULL, ckr, NULL, NULL, 0);

   node_walk(AST, tbl, 0, _checker_special, _checker_start, _checker_end);

   if (ckr->errors > 0) exit(-1);

   for (int i = 0; i < dy_len(ckr->scopes); i++) {
      dy_free(dyi(ckr->scopes)[i]->decls);
      free(dyi(ckr->scopes)[i]);
   }

   free(ckr->scopes);
   free(ckr);
}

bool checker_special(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope) {
   static checker_t *check = NULL;
   if (ckr != NULL) {
      check = ckr;

      return false;
   }

   switch (node->type) {
      case NODE_PATH_EXPRESSION: return true;
      default: return false;
   }
}

void checker_start(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   static checker_t *check = NULL;
   if (ckr != NULL) {
      check = ckr;

      return;
   }

   checker_scope_push(check, scope);

   switch (node->type) {
#define NODE(ident, ...) case NODE_##ident: return checker_##ident(node, check, tbl, mod, scope);
      NODE_TYPES
#undef NODE
      default: return;
   }
}

void checker_end(node_t *node, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   static checker_t *check = NULL;
   if (ckr != NULL) {
      check = ckr;

      return;
   }
}

void checker_scope_push(checker_t *ckr, size_t scope) {
   while (dy_len(ckr->scopes) <= scope) {
      ckr_scope_t *scp = malloc(sizeof(ckr_scope_t));

      scp->decls = dy_init(wchar_t *);
      scp->ret = -1;

      dy_push(ckr->scopes, scp);
   }
}

void checker_scope_set(checker_t *ckr, size_t scope, wchar_t *name) {
   while (dy_len(ckr->scopes) <= scope) {
      ckr_scope_t *scp = malloc(sizeof(ckr_scope_t));

      scp->decls = dy_init(wchar_t *);
      scp->ret = -1;

      dy_push(ckr->scopes, scp);
   }

   dy_push(dyi(ckr->scopes)[scope]->decls, name);
}

bool checker_scope_exists(checker_t *ckr, size_t scope, wchar_t *name) {
   if (dy_len(ckr->scopes) <= scope) return false;

   ckr_scope_t *scp = dyi(ckr->scopes)[scope];

   for (int i = 0; i < dy_len(scp->decls); i++)
      if (!wcscmp(dyi(scp->decls)[i], name)) return true;

   return false;
}

void checker_node(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   switch (n->type) {
#define NODE(ident, ...) case NODE_##ident: return checker_##ident(n, ckr, tbl, mod, scope);
      NODE_TYPES
#undef NODE
      default: return;
   }
}

// CHECKS

void checker_NONE(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_EMPTY(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_MULTI(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_BLOCK(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_PROJECT(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_NUMBER_LITERAL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_BOOL_LITERAL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_FLOAT_LITERAL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_STRING_LITERAL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }

void checker_STRUCT(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_ALIAS(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_UNINIT(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }

void checker_SOURCE(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_FILE(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }

void checker_TYPE_BASE(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }

void checker_TYPE_NAME(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, TYPE_NAME);

#define TYPE(ident, str, ...) if (!wcscmp(node->name, str)) return;
   BASE_TYPES
#undef TYPE

   if (sym_table_get(sym_table_get_module(tbl, node->module), node->name, scope, true) < 0)
      error("Unknown type: %ls!", node->name);
}

void checker_TYPE_PTR(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_TYPE_FUNCTION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }
void checker_TYPE_STRUCT(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {}

void checker_VARIABLE_DECLARATION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, VARIABLE_DECLARATION);

   if (checker_scope_exists(ckr, scope, node->ident->IDENTIFIER.value)) 
      error("Redeclaration of variable!");

   // TODO
   if (node->expr->type != NODE_NONE && node->type->type != NODE_NONE &&
       !type_cmp(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->type), infer_expression(tbl, mod, scope, node->expr)))
      error("Variable decl types do not match!");

   checker_scope_set(ckr, scope, node->ident->IDENTIFIER.value);
}

void checker_IF(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, IF);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->cond));
   if (type == NULL) return;

   if (!(type->type == TYPE_BASE && type->base == BASE_BOOL))
      error("If condition must be bool!");
}

void checker_FOR(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }

void checker_BREAK(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_CONTINUE(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }

void checker_RETURN(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, RETURN);

   type_idx retidx = dyi(ckr->scopes)[scope]->ret;

   if (retidx < 0) {
      size_t csco = scope;

      while ((csco = sym_table_get_parent(mod, csco)) != 0)
         if (dyi(ckr->scopes)[csco]->ret != -1) {
            retidx = dyi(ckr->scopes)[csco]->ret;

            break;
         }
   }

   if (!type_cmp(tbl->ty_hdl, retidx, node->value->type == NODE_NONE ? BASE_VOID : infer_expression(tbl, mod, scope, node->value)))
      error("Invalid return type!");
}

void checker_DEFER(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }

void checker_IMPL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, IMPL);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->type));
   if (type == NULL) return;

   if (type->type != TYPE_STRUCT) error("Impls may only be on structs!");
}

void checker_IDENTIFIER(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, IDENTIFIER);

   if (sym_table_get_both(mod, node->value, scope) < 0)
      error("Unknown identifier: %ls!", node->value);
}

void checker_STRUCT_LITERAL(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, STRUCT_LITERAL);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->type));
   if (type == NULL) return;

   if (type->type != TYPE_STRUCT) error("Struct literals must be of type struct!");

   if (dy_len(type->feilds) < dy_len(node->exprs)) error("Too many arguments to struct ltieral!");

   int type_pos = 0;
   for (int i = 0; i < dy_len(node->exprs); i++) {
      if (type_pos >= dy_len(type->feilds)) error("Too many arguments to struct ltieral!");

      type_idx ft = -1;

      if (dyi(node->idents)[i] == NULL) ft = dyi(type->feilds)[type_pos].type;
      else {
         int j = 0;
         for (; j < dy_len(type->feilds); j++)
            if (!wcscmp(dyi(type->feilds)[j].name, dyi(node->idents)[i])) {
               ft = dyi(type->feilds)[j].type;
               break;
            }

         if (ft < 0) error("Struct feild not found!");

         type_pos = j;
      }

      if (!type_cmp(tbl->ty_hdl, ft, infer_expression(tbl, mod, scope, dyi(node->exprs)[i])))
         error("Invalid struct arg type!");

      type_pos++;
   }
}

void checker_BINARY_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, BINARY_EXPRESSION);

   type_t *t1 = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->left));
   type_t *t2 = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->right));

   switch (node->op) {
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_ASTERISK:
      case TOKEN_FORWARD_SLASH:
      case TOKEN_MOD:
         if (!type_is_numeric(t1) || !type_is_numeric(t2)) 
            error("Operands must be numeric!");

         break;
      // case TOKEN_AND:
      // case TOKEN_AND_AND:
      // case TOKEN_OR:
      // case TOKEN_OR_OR:
      // case TOKEN_EQUALS_EQUALS:
      // case TOKEN_NOT_EQUALS:
      // case TOKEN_GREATER_THAN:
      // case TOKEN_GREATER_THAN_EQUALS:
      // case TOKEN_LESS_THAN:
      // case TOKEN_LESS_THAN_EQUALS:;
         // return BASE_BOOL;
   }
}

void checker_CALL_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, CALL_EXPRESSION);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->func));
   if (type == NULL) return;

   if (type->type != TYPE_FUNCTION) error("Invalid type being called!");

   if (dy_len(type->args) != dy_len(node->args))
      error("Invalid amount of arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_idx argt = infer_expression(tbl, mod, scope, dyi(node->args)[i]);
      type_idx typt = dyi(type->args)[i].type;

      if (!type_cmp(tbl->ty_hdl, typt, argt)) error("Invalid argument type!");
   }
}

void checker_PATH_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, PATH_EXPRESSION);

   sym_module_t *pmod = sym_table_get_module(tbl, node->module->IDENTIFIER.value);
   if (pmod == NULL) error("Unkown module!");

   if (node->expr->type == NODE_CALL_EXPRESSION) {
      node_def(n->PATH_EXPRESSION.expr, CALL_EXPRESSION);

      type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, pmod, scope, node->func));
      if (type == NULL) return;

      if (type->type != TYPE_FUNCTION) error("Invalid type being called!");

      if (dy_len(type->args) != dy_len(node->args))
         error("Invalid amount of arguments!");

      for (int i = 0; i < dy_len(node->args); i++) {
         type_idx argt = infer_expression(tbl, mod, scope, dyi(node->args)[i]);
         type_idx typt = dyi(type->args)[i].type;

         if (!type_cmp(tbl->ty_hdl, typt, argt)) error("Invalid argument type!");
      }

      for (int i = 0; i < dy_len(node->args); i++) 
         checker_node(dyi(node->args)[i], ckr, tbl, mod, scope);
   } else if (sym_table_get_both(pmod, node->expr->IDENTIFIER.value, 0) < 0)
      error("Unkown identifier!");
}

void checker_FEILD_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, FEILD_EXPRESSION);

   type_idx tidx = infer_expression(tbl, mod, scope, node->expr);
   type_t *type = type_get(tbl->ty_hdl, tidx);
   if (type == NULL) return;

   if (type->type == TYPE_PTR) {
      node->ptr = true;

      type = type_get(tbl->ty_hdl, type->ptr_base);
      if (type == NULL) return;
   }

   if (type->type != TYPE_STRUCT) error("Only structs have feilds!");

   for (int i = 0; i < dy_len(type->feilds); i++)
      if (!wcscmp(dyi(type->feilds)[i].name, node->member))
         return;

   error("Unkown feild!");
}

void checker_METHOD_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, METHOD_EXPRESSION);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->expr));
   if (type == NULL) return;

   if (type->type != TYPE_STRUCT) error("Only structs have methods!");

   type_idx funcidx = -1;
   for (int i = 0; i < dy_len(type->funcs); i++)
      if (!wcscmp(dyi(type->funcs)[i].name, node->member))
         funcidx = dyi(type->funcs)[i].type;

   if (funcidx < 0) error("Unknown method!");

   type_t *funct = type_get(tbl->ty_hdl, funcidx);

   if (dy_len(funct->args) != dy_len(node->args))
      error("Invalid amount of arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_idx argt = infer_expression(tbl, mod, scope, dyi(node->args)[i]);
      type_idx typt = dyi(funct->args)[i].type;

      if (!type_cmp(tbl->ty_hdl, typt, argt)) error("Invalid argument type!");
   }
}

void checker_SUBSCRIPT_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_SIGN_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_INCDEC_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_NOT_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_DEREF_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_ADDR_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { ERROR("UNIMPLEMENTED!"); }
void checker_CAST_EXPRESSION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) { }

// TODO: man the checker scope system is fucking ass fix that shit
void checker_FUNCTION_DECLARATION(node_t *n, checker_t *ckr, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(n, FUNCTION_DECLARATION);

   type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->type));
   if (type == NULL) return;

   dyi(ckr->scopes)[scope]->ret = type->ret;
}

#include "checker.h"

#define error(msg) ({ ckr->errors++; printf("%s\n", msg); return false; })
#define is_typedef(node) (M_COMPARE(node->type, NODE_STRUCT, NODE_ALIAS))

bool checker_check_NONE(checker_t *ckr, node_t *n)           { return true; }
bool checker_check_EMPTY(checker_t *ckr, node_t *n)          { return true; }
bool checker_check_MULTI(checker_t *ckr, node_t *n)          { return true; }
bool checker_check_NUMBER_LITERAL(checker_t *ckr, node_t *n) { return true; }
bool checker_check_FLOAT_LITERAL(checker_t *ckr, node_t *n)  { return true; }
bool checker_check_STRING_LITERAL(checker_t *ckr, node_t *n) { return true; }
bool checker_check_ALIAS(checker_t *ckr, node_t *n)          { return true; }
bool checker_check_UNINIT(checker_t *ckr, node_t *n)         { return true; }

bool checker_check_FILE(checker_t *ckr, node_t *n) {
   node_def(n, FILE);

   for (int i = 0; i < dy_len(node->stmts); i++) {
      node_t *stmt = dyi(node->stmts)[i];

      switch (stmt->type) {
         case NODE_VARIABLE_DECLARATION:
            if (checker_decl_exists_cur(ckr, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value)) error("Redeclaration of Variable!");

            checker_set_decl(
               ckr, 
               stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value, 
               (decl_t) { 
                  .is_typedef = is_typedef(stmt->VARIABLE_DECLARATION.expr), 
                  .type = checker_infer_var_decl(ckr, stmt) 
               }
            );
            break;
         case NODE_EMPTY: continue;
         default: error("Statment Cant be File Scope!");
      }
   }

   return true;
}

bool checker_check_BLOCK(checker_t *ckr, node_t *n) {
   scope_t *scope = malloc(sizeof(scope_t));

   scope->decls = ht_init_sv(wchar_t, decl_t *);
   scope->parent = ckr->cur_scope;
   scope->ret = ckr->cur_scope->ret;

   ckr->cur_scope = scope;

   return true;
}

bool checker_check_BLOCK_end(checker_t *ckr, node_t *n) {
   scope_t *scope = ckr->cur_scope;

   ht_free_func(scope->decls, free_decl);
   ckr->cur_scope = scope->parent;

   free(scope);

   return true;
}

bool checker_check_BINARY_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, BINARY_EXPRESSION);

   type_t *left  = checker_infer_expression(ckr, node->left);
   type_t *right = checker_infer_expression(ckr, node->right);

   switch (node->op) {
      case TOKEN_EQUALS_EQUALS:
      case TOKEN_NOT_EQUALS:
      case TOKEN_AND_AND:
      case TOKEN_OR_OR:
      case TOKEN_GREATER_THAN:
      case TOKEN_LESS_THAN:
         return true;
      default: return true;
   }

   return true;
}

bool checker_check_CALL_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, CALL_EXPRESSION);

   type_t *type = checker_infer_callexpr_funct(ckr, n);
   if (type == NULL) return false;

   if (dy_len(type->args) != dy_len(node->args)) 
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_infer_expression(ckr, dyi(node->args)[i]);
      type_t *typt = checker_reslove_type(ckr, dyi(type->args)[i].type);

      if (!type_cmp(ckr, typt, argt)) error("Invalid Argument Type!");
   }

   return true;
}

bool checker_check_SUBSCRIPT_EXPRESSION(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_SIGN_EXPRESSION(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_INCDEC_EXPRESSION(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_NOT_EXPRESSION(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_DEREF_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, DEREF_EXPRESSION);

   if (!type_is_ptr(checker_infer_expression(ckr, node->expr))) 
      error("Cannot Derefrence Non Ptr Type!");

   return true;
}

bool checker_check_ADDR_EXPRESSION(checker_t *ckr, node_t *n) {
   // ERROR("UNIMPLEMENTED!");
   // TODO:

   return true;
}

bool checker_check_CAST_EXPRESSION(checker_t *ckr, node_t *n) {
   // ERROR("UNIMPLEMENTED!");
   return true;
}

bool checker_check_ACCESS_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, ACCESS_EXPRESSION);

   type_t *type = checker_infer_expression(ckr, node->expr);
   if (type == NULL) return false;

   if (type->type == TYPE_PTR) 
      n->ACCESS_EXPRESSION.ptr = true;

   return true;
}

bool checker_check_STRUCT(checker_t *ckr, node_t *n) {
   node_def(n, STRUCT);

   type_t *type = checker_infer_expression(ckr, node->type);

   for (int i = 0; i < dy_len(type->feilds); i++) {
      for (int j = i - 1; j >= 0; j--)
         if (!wcscmp(dyi(type->feilds)[i].name, dyi(type->feilds)[j].name))
            error("Duplicate Name in Struct!");

      type_t *fet = checker_reslove_type(ckr, dyi(type->feilds)[i].type);
      if (fet == NULL) error("Unknown Type!");
   }

   return true;
}

bool checker_check_IDENTIFIER(checker_t *ckr, node_t *n) {
   node_def(n, IDENTIFIER);

   if (checker_get_decl(ckr, node->value) == NULL) {
      print(node->value);
      error("Unknown Identifier!");
   }

   return true;
}

bool checker_check_DATA_TYPE(checker_t *ckr, node_t *n) {
   node_def(n, DATA_TYPE);

   type_t *type = checker_reslove_base_type(ckr, node->type);
   if (type == NULL) error("Unknown Type!");

   if (type->type == TYPE_FUNCTION)
      for (int i = 0; i < dy_len(type->args); i++)
         if (checker_reslove_base_type(ckr, dyi(type->args)[i].type) == NULL) error("Unknown Type!");

   return true;
}

bool checker_check_VARIABLE_DECLARATION(checker_t *ckr, node_t *n) {
   node_def(n, VARIABLE_DECLARATION);

   wchar_t *name    = node->ident->IDENTIFIER.value;
   type_t *type  = checker_reslove_type(ckr, node->type->DATA_TYPE.type);
   if (type == NULL) return false;

   if (ckr->cur_scope != ckr->file_scope && checker_decl_exists_cur(ckr, name)) error("Redeclaration of Variable!");

   if (!(node->expr->type == NODE_NONE || node->expr->type == NODE_UNINIT)) {
      type_t *infer = checker_infer_expression(ckr, node->expr);
      if (infer == NULL) return false;

      if (type->type == TYPE_INFER) {
         infer = checker_infer_expression_no_res(ckr, node->expr);
         if (infer->type == TYPE_UNTYPED) infer = infer->uninfer; 

         node->type->DATA_TYPE.type = infer;
      } else if (!type_cmp(ckr, type, infer)) error("Variable Declaration Types do Not Match!");
   }

   checker_set_decl(ckr, name, (decl_t) { .is_typedef = is_typedef(node->expr), .type = node->type->DATA_TYPE.type });

   return true;
}

bool checker_check_FUNCTION_DECLARATION(checker_t *ckr, node_t *n) {
   node_def(n, FUNCTION_DECLARATION);

   type_t *type = node->type->DATA_TYPE.type;

   scope_t *scope = malloc(sizeof(scope_t));

   scope->decls = ht_init_sv(wchar_t, decl_t *);
   scope->parent = ckr->cur_scope;
   scope->ret = type->ret;

   ckr->cur_scope = scope;

   for (int i = 0; i < dy_len(type->args); i++) {
      if (checker_decl_exists_cur(ckr, dyi(type->args)[i].name)) error("Redefiation of Variable!");

      checker_set_decl(ckr, dyi(type->args)[i].name, (decl_t) { .type = dyi(type->args)[i].type });
   }

   return true;
}

bool checker_check_FUNCTION_DECLARATION_end(checker_t *ckr, node_t *n) {
   scope_t *scope = ckr->cur_scope;

   ht_free_func(scope->decls, free_decl);
   ckr->cur_scope = scope->parent;

   free(scope);

   return true;
}

bool checker_check_IF(checker_t *ckr, node_t *n) {
   node_def(n, IF);

   type_t *type = checker_infer_expression(ckr, node->cond);

   if (type->type == TYPE_BASE && type->base == BASE_BOOL)
      return true;
   else error("If condition must be bool!");
}

bool checker_check_FOR(checker_t *ckr, node_t *n) {
   node_def(n, FOR);

   type_t *type = checker_infer_expression(ckr, node->cond);

   if (type->type == TYPE_BASE && type->base == BASE_BOOL)
      return true;
   else error("For condition must be bool!");
}

bool checker_check_BREAK(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_CONTINUE(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_RETURN(checker_t *ckr, node_t *n) {
   node_def(n, RETURN);

   if (!type_cmp(ckr, ckr->cur_scope->ret, checker_infer_expression(ckr, node->value)))
      error("Invalid Return Type!");

   return true;
}
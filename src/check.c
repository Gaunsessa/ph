#include "checker.h"

#define error(msg) ({ ckr->errors++; printf("%s\n", msg); return false; })
#define is_typedef(node) (M_COMPARE(node->type, NODE_STRUCT, NODE_ALIAS))

bool checker_check_NONE(checker_t *ckr, module_t *mod, node_t *n)           { return true; }
bool checker_check_EMPTY(checker_t *ckr, module_t *mod, node_t *n)          { return true; }
bool checker_check_MULTI(checker_t *ckr, module_t *mod, node_t *n)          { return true; }
bool checker_check_NUMBER_LITERAL(checker_t *ckr, module_t *mod, node_t *n) { return true; }
bool checker_check_FLOAT_LITERAL(checker_t *ckr, module_t *mod, node_t *n)  { return true; }
bool checker_check_STRING_LITERAL(checker_t *ckr, module_t *mod, node_t *n) { return true; }
bool checker_check_ALIAS(checker_t *ckr, module_t *mod, node_t *n)          { return true; }
bool checker_check_UNINIT(checker_t *ckr, module_t *mod, node_t *n)         { return true; }
bool checker_check_DEFER(checker_t *ckr, module_t *mod, node_t *n)          { return true; }

bool checker_check_SOURCE(checker_t *ckr, module_t *mod, node_t *n)         { unreachable(); }

bool checker_check_PROJECT(checker_t *ckr, module_t *mod, node_t *n) {
   return true;
}

bool checker_check_FILE(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, FILE);

   checker_push_scope(mod);

   mod->file_scope = mod->cur_scope;

   // TODO: This will cause a double free when type freeing is implemented!
   // TODO: parse if import is real
   for (int i = 0; i < dy_len(node->imports); i++)
      checker_set_decl(
         mod, 
         dyi(node->imports)[i], 
         type_init((type_t) { TYPE_MODULE, .name = dyi(node->imports)[i] }), 
         false
      );

   for (int i = 0; i < dy_len(node->stmts); i++) {
      node_t *stmt = dyi(node->stmts)[i];

      switch (stmt->type) {
         case NODE_VARIABLE_DECLARATION:
            if (checker_decl_exists_cur(mod, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value)) error("Redeclaration of Variable!");

            checker_set_decl(
               mod, 
               stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value, 
               checker_infer_var_decl(ckr, mod, stmt),
               is_typedef(stmt->VARIABLE_DECLARATION.expr)
            );
            break;
         case NODE_IMPL:;
            type_t *type = checker_resolve_base_type(mod, checker_infer_expression(ckr, mod, stmt->IMPL.type));
            if (type == NULL) return false;

            if (type->type != TYPE_STRUCT) error("Impls can only be done on structs!");

            for (int i = 0; i < dy_len(stmt->IMPL.funcs); i++) {
               node_t *func = dyi(stmt->IMPL.funcs)[i];

               func->VARIABLE_DECLARATION.expr->FUNCTION_DECLARATION.type->DATA_TYPE.type->self.type = type_init((type_t) { .type = TYPE_PTR, .ptr_base = stmt->IMPL.type->DATA_TYPE.type});

               struct { wchar_t *name; struct type_t *type; } ft = {
                  .name = func->VARIABLE_DECLARATION.ident->IDENTIFIER.value,
                  .type = checker_infer_var_decl(ckr, mod, func)
               };

               dy_push_unsafe(type->funcs, ft);
            }

            continue;
         case NODE_EMPTY: continue;
         default: error("Statment Cant be File Scope!");
      }
   }

   return true;
}

bool checker_check_BLOCK(checker_t *ckr, module_t *mod, node_t *n) {
   checker_push_scope(mod);

   return true;
}

bool checker_check_BINARY_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, BINARY_EXPRESSION);

   type_t *left  = checker_infer_expression(ckr, mod, node->left);
   type_t *right = checker_infer_expression(ckr, mod, node->right);

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

bool checker_check_CALL_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, CALL_EXPRESSION);

   type_t *type = checker_infer_callexpr_funct(ckr, mod, n);
   if (type == NULL) return false;

   if (type->type != TYPE_FUNCTION) error("Invalid type being called!");

   if (dy_len(type->args) != dy_len(node->args))
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_infer_expression(ckr, mod, dyi(node->args)[i]);
      type_t *typt = checker_resolve_type(mod, dyi(type->args)[i].type);

      if (!type_cmp(mod, typt, argt)) error("Invalid Argument Type!");
   }

   return true;
}

bool checker_check_SUBSCRIPT_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_SIGN_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   // ERROR("UNIMPLEMENTED!");
}

bool checker_check_INCDEC_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_NOT_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_DEREF_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, DEREF_EXPRESSION);

   if (!type_is_ptr(checker_infer_expression(ckr, mod, node->expr))) 
      error("Cannot Derefrence Non Ptr Type!");

   return true;
}

bool checker_check_ADDR_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   // ERROR("UNIMPLEMENTED!");
   // TODO:

   return true;
}

bool checker_check_CAST_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   // ERROR("UNIMPLEMENTED!");
   return true;
}

bool checker_check_PATH_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, PATH_EXPRESSION);

   if (node->expr->type == NODE_IDENTIFIER) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) error("Unkown import!");

      if (checker_get_decl_both(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->IDENTIFIER.value) == NULL)
         error("Unknown identifier on module!");
   } else if (node->expr->type == NODE_CALL_EXPRESSION) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) error("Unkown import!");

      type_t *func = checker_get_decl(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->CALL_EXPRESSION.func->IDENTIFIER.value, false);

      if (func == NULL) error("Unkown method!");

      if (dy_len(func->args) != dy_len(node->expr->CALL_EXPRESSION.args))
         error("Invalid Amount of Arguments!");

      for (int i = 0; i < dy_len(node->expr->CALL_EXPRESSION.args); i++) {
         type_t *argt = checker_infer_expression(ckr, mod, dyi(node->expr->CALL_EXPRESSION.args)[i]);
         type_t *typt = checker_resolve_type(mod, dyi(func->args)[i].type);

         if (!type_cmp(mod, typt, argt)) error("Invalid Argument Type!");
      }
   } else error("Path exprssions can only be idents and funcs!");

   return true;
}

bool checker_check_FEILD_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, FEILD_EXPRESSION);

   type_t *oty  = checker_infer_expression(ckr, mod, node->expr);
   type_t *type = checker_resolve_base_type(mod, oty);
   if (oty == NULL || type == NULL) return false;

   if (type->type == TYPE_STRUCT) {
      bool found = false;
      for (int i = 0; i < dy_len(type->feilds); i++)
         if (!wcscmp(dyi(type->feilds)[i].name, node->member))
            found = true;

      if (!found) error("Unknown feild!");

      if (oty->type == TYPE_PTR) 
         n->FEILD_EXPRESSION.ptr = true;
   } else error("Only structs have feilds!");

   return true;
}

bool checker_check_METHOD_EXPRESSION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, METHOD_EXPRESSION);

   type_t *oty  = checker_infer_expression(ckr, mod, node->expr);
   type_t *type = checker_resolve_base_type(mod, oty);
   if (oty == NULL || type == NULL) return false;

   type_t *func = NULL;
   if (type->type == TYPE_STRUCT) {
      for (int i = 0; i < dy_len(type->funcs); i++)
         if (!wcscmp(dyi(type->funcs)[i].name, node->member))
            func = dyi(type->funcs)[i].type;

      if (oty->type == TYPE_PTR)
            n->METHOD_EXPRESSION.ptr = true;
   } else error("Only structs and modules have feilds!");

   if (func == NULL) error("Unkown method!");

   if (dy_len(func->args) != dy_len(node->args))
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_infer_expression(ckr, mod, dyi(node->args)[i]);
      type_t *typt = checker_resolve_type(mod, dyi(func->args)[i].type);

      if (!type_cmp(mod, typt, argt)) error("Invalid Argument Type!");
   }

   return true;
}

bool checker_check_STRUCT(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, STRUCT);

   type_t *type = checker_infer_expression(ckr, mod, node->type);

   for (int i = 0; i < dy_len(type->feilds); i++) {
      for (int j = i - 1; j >= 0; j--)
         if (!wcscmp(dyi(type->feilds)[i].name, dyi(type->feilds)[j].name))
            error("Duplicate Name in Struct!");

      type_t *fet = checker_resolve_type(mod, dyi(type->feilds)[i].type);
      if (fet == NULL) error("Unknown Type!");
   }

   return true;
}

bool checker_check_IDENTIFIER(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, IDENTIFIER);

   if (checker_get_decl(mod, node->value, false) == NULL) {
      printf("%ls\n", node->value);
      error("Unknown Identifier!");
   }

   return true;
}

bool checker_check_DATA_TYPE(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, DATA_TYPE);

   type_t *type = checker_resolve_base_type(mod, checker_resolve_type(mod, node->type));
   if (type == NULL) error("Unknown Type!");

   if (type->type == TYPE_FUNCTION)
      for (int i = 0; i < dy_len(type->args); i++)
         if (checker_resolve_base_type(mod, dyi(type->args)[i].type) == NULL) error("Unknown Type!");

   return true;
}

bool checker_check_VARIABLE_DECLARATION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, VARIABLE_DECLARATION);

   wchar_t *name = node->ident->IDENTIFIER.value;
   type_t *type  = checker_resolve_type(mod, checker_infer_expression(ckr, mod, node->type));
   if (type == NULL) return false;

   if (mod->cur_scope != mod->file_scope && checker_decl_exists_cur(mod, name)) error("Redeclaration of Variable!");

   if (!(node->expr->type == NODE_NONE || node->expr->type == NODE_UNINIT)) {
      type_t *infer = checker_infer_expression(ckr, mod, node->expr);
      if (infer == NULL) return false;

      if (type->type == TYPE_INFER) {
         infer = checker_infer_expression_no_res(ckr, mod, node->expr);
         if (infer->type == TYPE_UNTYPED) infer = infer->uninfer; 

         node->type->DATA_TYPE.type = infer;

         printf("%ls: ", name);
         print(infer->type);
      } else if (!type_cmp(mod, type, infer)) error("Variable Declaration Types do Not Match!");
   }

   checker_set_decl(mod, name, checker_infer_expression_no_res(ckr, mod, node->type), is_typedef(node->expr));

   return true;
}

bool checker_check_FUNCTION_DECLARATION(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, FUNCTION_DECLARATION);

   type_t *type = checker_infer_expression(ckr, mod, node->type);

   checker_push_scope(mod);

   mod->cur_scope->ret = type->ret;

   if (type->self.name != NULL) checker_set_decl(mod, type->self.name, type->self.type, false);

   for (int i = 0; i < dy_len(type->args); i++) {
      if (checker_decl_exists_cur(mod, dyi(type->args)[i].name)) error("Redefiation of Variable!");

      checker_set_decl(mod, dyi(type->args)[i].name, dyi(type->args)[i].type, false);
   }

   return true;
}

bool checker_check_IF(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, IF);

   type_t *type = checker_infer_expression(ckr, mod, node->cond);

   if (type->type == TYPE_BASE && type->base == BASE_BOOL)
      return true;
   else error("If condition must be bool!");
}

bool checker_check_FOR(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, FOR);

   checker_push_scope(mod);

   // This needs to be checked before
   checker_check_node(ckr, mod, node->init);

   if (node->cond->type != NODE_NONE) {
      type_t *type = checker_infer_expression(ckr, mod, node->cond);

      // TODO: for needs to make scope

      if (type->type == TYPE_BASE && type->base == BASE_BOOL)
         return true;
      else error("For condition must be bool!");
   } else return true;
}

bool checker_check_BREAK(checker_t *ckr, module_t *mod, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_CONTINUE(checker_t *ckr, module_t *mod, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_RETURN(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, RETURN);

   if (node->value->type == NODE_NONE) {
      if (mod->cur_scope->ret->type == TYPE_BASE && mod->cur_scope->ret->base != BASE_VOID)
         error("Invalid Return Type!");
   } else if (!type_cmp(mod, mod->cur_scope->ret, checker_infer_expression(ckr, mod, node->value)))
      error("Invalid Return Type!");

   return true;
}

bool checker_check_IMPL(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, IMPL);

   checker_push_scope(mod);

   type_t *type = checker_resolve_base_type(mod, checker_infer_expression(ckr, mod, node->type));
   if (type == NULL) return false;

   if (type->type != TYPE_STRUCT) error("Impls can only be done on structs!");

   // for (int i = 0; i < dy_len(node->funcs); i++) {
   //    node_t *func = dyi(node->funcs)[i];

   //    if (func->VARIABLE_DECLARATION.expr->type != NODE_FUNCTION_DECLARATION)
   //       error("Impl can only have functions!");

   //    func->VARIABLE_DECLARATION.expr->FUNCTION_DECLARATION.type->DATA_TYPE.type->self.type = type_init((type_t) { .type = TYPE_PTR, .ptr_base = node->type->DATA_TYPE.type});

   //    struct { wchar_t *name; struct type_t *type; } ft = {
   //       .name = func->VARIABLE_DECLARATION.ident->IDENTIFIER.value,
   //       .type = checker_infer_var_decl(ckr, mod, func)
   //    };

   //    dy_push_unsafe(type->funcs, ft);
   // }

   return true;
}

bool checker_check_STRUCT_LITERAL(checker_t *ckr, module_t *mod, node_t *n) {
   node_def(n, STRUCT_LITERAL);

   type_t *type = checker_resolve_base_type(mod, checker_infer_expression(ckr, mod, node->type));
   if (type == NULL) return false;

   if (type->type != TYPE_STRUCT) error("Struct literals must be of type struct!");

   if (dy_len(type->feilds) < dy_len(node->exprs)) error("Too many arguments to struct ltieral!");

   int type_pos = 0;
   for (int i = 0; i < dy_len(node->exprs); i++) {
      if (type_pos >= dy_len(type->feilds)) error("Too many arguments to struct ltieral!");

      type_t *ft = NULL;

      if (dyi(node->idents)[i] == NULL) ft = dyi(type->feilds)[type_pos].type;
      else {
         int j = 0;
         for (; j < dy_len(type->feilds); j++)
            if (!wcscmp(dyi(type->feilds)[j].name, dyi(node->idents)[i])) {
               ft = dyi(type->feilds)[j].type;
               break;
            }

         if (ft == NULL) error("Struct feild not found!");

         type_pos = j;
      }

      if (!type_cmp(mod, ft, checker_infer_expression(ckr, mod, dyi(node->exprs)[i]))) 
         error("Invalid struct arg type!");

      type_pos++;
   }

   return true;
}

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
bool checker_check_DEFER(checker_t *ckr, node_t *n)          { return true; }

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
               checker_infer_var_decl(ckr, stmt),
               is_typedef(stmt->VARIABLE_DECLARATION.expr)
            );
            break;
         case NODE_IMPL:
         case NODE_EMPTY: continue;
         default: error("Statment Cant be File Scope!");
      }
   }

   return true;
}

bool checker_check_BLOCK(checker_t *ckr, node_t *n) {
   checker_push_scope(ckr);

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
   // ERROR("UNIMPLEMENTED!");
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

bool checker_check_FEILD_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, FEILD_EXPRESSION);

   type_t *oty  = checker_infer_expression(ckr, node->expr);
   type_t *type = checker_reslove_base_type(ckr, oty);
   if (oty == NULL || type == NULL) return false;

   if (type->type != TYPE_STRUCT) error("Only structs have feilds!");

   if (oty->type == TYPE_PTR) 
      n->FEILD_EXPRESSION.ptr = true;

   // for (int i = 0; i < dy_len(type->funcs); i++)
   //    checker_set_decl(ckr, dyi(type->funcs)[i].name, dyi(type->funcs)[i].type, false);

   // if (node->member->type == NODE_CALL_EXPRESSION)
   //    checker_check_CALL_EXPRESSION(ckr, node->member);

   // if (node->member->type == NODE_CALL_EXPRESSION)
   //    dy_insert(node->member->CALL_EXPRESSION.args, 0, node->expr);

   return true;
}

bool checker_check_METHOD_EXPRESSION(checker_t *ckr, node_t *n) {
   node_def(n, METHOD_EXPRESSION);

   type_t *oty  = checker_infer_expression(ckr, node->expr);
   type_t *type = checker_reslove_base_type(ckr, oty);
   if (oty == NULL || type == NULL) return false;

   if (type->type != TYPE_STRUCT) error("Only structs have methods!");

   type_t *func = NULL;
   for (int i = 0; i < dy_len(type->funcs); i++)
      if (!wcscmp(dyi(type->funcs)[i].name, node->member))
         func = dyi(type->funcs)[i].type;

   if (func == NULL) error("Unkown method!");

   if (dy_len(func->args) != dy_len(node->args))
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_infer_expression(ckr, dyi(node->args)[i]);
      type_t *typt = checker_reslove_type(ckr, dyi(func->args)[i].type);

      if (!type_cmp(ckr, typt, argt)) error("Invalid Argument Type!");
   }

   if (oty->type == TYPE_PTR)
      n->METHOD_EXPRESSION.ptr = true;

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

   if (checker_get_decl(ckr, node->value, false) == NULL) {
      printf("%ls\n", node->value);
      error("Unknown Identifier!");
   }

   return true;
}

bool checker_check_DATA_TYPE(checker_t *ckr, node_t *n) {
   node_def(n, DATA_TYPE);

   type_t *type = checker_reslove_base_type(ckr, checker_reslove_type(ckr, node->type));
   if (type == NULL) error("Unknown Type!");

   if (type->type == TYPE_FUNCTION)
      for (int i = 0; i < dy_len(type->args); i++)
         if (checker_reslove_base_type(ckr, dyi(type->args)[i].type) == NULL) error("Unknown Type!");

   return true;
}

bool checker_check_VARIABLE_DECLARATION(checker_t *ckr, node_t *n) {
   node_def(n, VARIABLE_DECLARATION);

   wchar_t *name    = node->ident->IDENTIFIER.value;
   type_t *type     = checker_reslove_type(ckr, node->type->DATA_TYPE.type);
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

   checker_set_decl(ckr, name, node->type->DATA_TYPE.type, is_typedef(node->expr));

   return true;
}

bool checker_check_FUNCTION_DECLARATION(checker_t *ckr, node_t *n) {
   node_def(n, FUNCTION_DECLARATION);

   type_t *type = node->type->DATA_TYPE.type;

   checker_push_scope(ckr);

   ckr->cur_scope->ret = type->ret;

   if (type->self.name != NULL) checker_set_decl(ckr, type->self.name, type->self.type, false);

   for (int i = 0; i < dy_len(type->args); i++) {
      if (checker_decl_exists_cur(ckr, dyi(type->args)[i].name)) error("Redefiation of Variable!");

      checker_set_decl(ckr, dyi(type->args)[i].name, dyi(type->args)[i].type, false);
   }

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

   checker_push_scope(ckr);

   // This needs to be checked before
   checker_check_node(ckr, node->init);

   if (node->cond->type != NODE_NONE) {
      type_t *type = checker_infer_expression(ckr, node->cond);

      // TODO: for needs to make scope

      if (type->type == TYPE_BASE && type->base == BASE_BOOL)
         return true;
      else error("For condition must be bool!");
   } else return true;
}

bool checker_check_BREAK(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_CONTINUE(checker_t *ckr, node_t *n) {
   ERROR("UNIMPLEMENTED!");
}

bool checker_check_RETURN(checker_t *ckr, node_t *n) {
   node_def(n, RETURN);


   if (node->value->type == NODE_NONE) {
      if (ckr->cur_scope->ret->type == TYPE_BASE && ckr->cur_scope->ret->base != BASE_VOID)
         error("Invalid Return Type!");
   } else if (!type_cmp(ckr, ckr->cur_scope->ret, checker_infer_expression(ckr, node->value)))
      error("Invalid Return Type!");

   return true;
}

bool checker_check_IMPL(checker_t *ckr, node_t *n) {
   node_def(n, IMPL);

   type_t *type = checker_reslove_base_type(ckr, checker_reslove_type(ckr, node->type->DATA_TYPE.type));
   if (type == NULL) return false;

   if (type->type != TYPE_STRUCT) error("Impls can only be done on structs!");

   for (int i = 0; i < dy_len(node->funcs); i++) {
      node_t *func = dyi(node->funcs)[i];

      if (func->VARIABLE_DECLARATION.expr->type != NODE_FUNCTION_DECLARATION)
         error("Impl can only have functions!");

      func->VARIABLE_DECLARATION.expr->FUNCTION_DECLARATION.type->DATA_TYPE.type->self.type = type_init((type_t) { .type = TYPE_PTR, .ptr_base = node->type->DATA_TYPE.type});

      struct { wchar_t *name; struct type_t *type; } ft = {
         .name = func->VARIABLE_DECLARATION.ident->IDENTIFIER.value,
         .type = checker_infer_var_decl(ckr, func)
      };

      dy_push_unsafe(type->funcs, ft);
   }

   checker_push_scope(ckr);

   return true;
}

bool checker_check_STRUCT_LITERAL(checker_t *ckr, node_t *n) {
   node_def(n, STRUCT_LITERAL);

   type_t *type = checker_reslove_base_type(ckr, checker_reslove_type(ckr, node->type->DATA_TYPE.type));
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

      if (!type_cmp(ckr, ft, checker_infer_expression(ckr, dyi(node->exprs)[i]))) 
         error("Invalid struct arg type!");

      type_pos++;
   }

   return true;
}

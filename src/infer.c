#include "checker.h"

type_t *checker_infer_expression(checker_t *ckr, module_t *mod, node_t *expr) {
   return checker_resolve_type(mod, checker_infer_expression_no_res(ckr, mod, expr));
}

type_t *checker_infer_expression_no_res(checker_t *ckr, module_t *mod, node_t *expr) {
   switch (expr->type) {
      case NODE_BINARY_EXPRESSION: return checker_infer_binexpr(ckr, mod, expr);
      case NODE_CAST_EXPRESSION: return checker_infer_castexpr(ckr, mod, expr);
      case NODE_DEREF_EXPRESSION: return checker_infer_derefexpr(ckr, mod, expr);
      case NODE_CALL_EXPRESSION: return checker_infer_callexpr(ckr, mod, expr);
      case NODE_PATH_EXPRESSION: return checker_infer_pathexpr(ckr, mod, expr);
      case NODE_FEILD_EXPRESSION: return checker_infer_feildexpr(ckr, mod, expr);
      case NODE_METHOD_EXPRESSION: return checker_infer_methodexpr(ckr, mod, expr);
      case NODE_NUMBER_LITERAL:
      case NODE_FLOAT_LITERAL:
      case NODE_STRING_LITERAL: return checker_infer_literal(ckr, mod, expr);
      case NODE_IDENTIFIER: return checker_infer_identifier(ckr, mod, expr);
      case NODE_FUNCTION_DECLARATION: return expr->FUNCTION_DECLARATION.type->DATA_TYPE.type;
      case NODE_STRUCT: return checker_infer_expression_no_res(ckr, mod, expr->STRUCT.type);
      case NODE_DATA_TYPE: return expr->DATA_TYPE.type;
      case NODE_STRUCT_LITERAL: return checker_infer_expression_no_res(ckr, mod, expr->STRUCT_LITERAL.type);
      case NODE_ADDR_EXPRESSION: return checker_infer_addrexpr(ckr, mod, expr);
      case NODE_ALIAS: return checker_infer_alias(ckr, mod, expr);
      case NODE_SIGN_EXPRESSION: return checker_infer_expression_no_res(ckr, mod, expr->SIGN_EXPRESSION.expr);

      default: eprint("Invalid Expression!", expr->type);
   }
}

type_t *checker_infer_binexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, BINARY_EXPRESSION);

   type_t *left  = checker_infer_expression_no_res(ckr, mod, node->left);
   type_t *right = checker_infer_expression_no_res(ckr, mod, node->right);

   switch (node->op) {
      case TOKEN_EQUALS_EQUALS:
      case TOKEN_NOT_EQUALS:
      case TOKEN_AND_AND:
      case TOKEN_OR_OR:
      case TOKEN_GREATER_THAN:
      case TOKEN_LESS_THAN:
      case TOKEN_GREATER_THAN_EQUALS:
      case TOKEN_LESS_THAN_EQUALS:
         return ht_get(BASE_TYPE_ENUM_VALUES, BASE_BOOL);
      default:
         if (left->type == TYPE_UNTYPED) {
            if (right->type == TYPE_UNTYPED)
               return (left->uninfer->base == BASE_F32 || right->uninfer->base == BASE_F32) ? BASE_UNTYPED_FLOAT : BASE_UNTYPED_INT;
            else return right;
         } else return left;
   }
}

type_t *checker_infer_pathexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, PATH_EXPRESSION);

   if (node->expr->type == NODE_IDENTIFIER) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) return NULL;

      type_t *type = checker_get_decl_both(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->IDENTIFIER.value);

      return type;
   } else if (node->expr->type == NODE_CALL_EXPRESSION) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) return NULL;

      type_t * type = checker_get_decl(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->CALL_EXPRESSION.func->IDENTIFIER.value, false);

      if (type == NULL || type->type != TYPE_FUNCTION) return NULL;

      return type->ret;
   }

   return NULL;
}

type_t *checker_infer_feildexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, FEILD_EXPRESSION);

   type_t *type = checker_resolve_base_type(mod, checker_infer_expression_no_res(ckr, mod, node->expr));
   if (type == NULL) return NULL;

   if (type->type == TYPE_STRUCT)
      for (int i = 0; i < dy_len(type->feilds); i++)
         if (!wcscmp(dyi(type->feilds)[i].name, node->member)) 
            return dyi(type->feilds)[i].type;

   return NULL;
}

type_t *checker_infer_methodexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, METHOD_EXPRESSION);

   type_t *type = checker_resolve_base_type(mod, checker_infer_expression_no_res(ckr, mod, node->expr));
   if (type == NULL) return NULL;

   type_t *funct = NULL;

   if (type->type == TYPE_STRUCT)
      for (int i = 0; i < dy_len(type->funcs); i++)
         if (!wcscmp(dyi(type->funcs)[i].name, node->member))
            funct = dyi(type->funcs)[i].type;

   if (funct == NULL || funct->type != TYPE_FUNCTION) return NULL;

   return funct->ret;
}

type_t *checker_infer_addrexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, ADDR_EXPRESSION);

   type_t *type = checker_infer_expression_no_res(ckr, mod, node->expr);

   return type_init((type_t) { TYPE_PTR, .ptr_base = type });
}

type_t *checker_infer_derefexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, DEREF_EXPRESSION);

   type_t *type = checker_infer_expression_no_res(ckr, mod, node->expr);

   return type->type == TYPE_PTR ? type->ptr_base : NULL;
}

type_t *checker_infer_castexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CAST_EXPRESSION);

   return node->type->DATA_TYPE.type;
}

type_t *checker_infer_callexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_resolve_type(mod, checker_infer_identifier(ckr, mod, node->func));
   // print(ctype == NULL);
   // printf("%d: %ls\n", ctype == NULL, node->func->IDENTIFIER.value);
   if (ctype == NULL) return NULL;

   return ctype->ret;
}

type_t *checker_infer_callexpr_funct(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_resolve_type(mod, checker_infer_identifier(ckr, mod, node->func));
   if (ctype == NULL) return NULL;

   return ctype;
}

type_t *checker_infer_literal(checker_t *ckr, module_t *mod, node_t *lit) {
   switch (lit->type) {
      case NODE_STRING_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_STRING);
      case NODE_FLOAT_LITERAL: return BASE_UNTYPED_FLOAT;
      case NODE_NUMBER_LITERAL: return BASE_UNTYPED_INT;
      default: eprint("Invalid Literal!");
   }
}

type_t *checker_infer_identifier(checker_t *ckr, module_t *mod, node_t *ident) {
   node_def(ident, IDENTIFIER);

   type_t *type = checker_get_decl(mod, node->value, false);
   if (type == NULL) return NULL;

   return type;
}

type_t *checker_infer_var_decl(checker_t *ckr, module_t *mod, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   if (node->type->DATA_TYPE.type->type != TYPE_INFER)
      return node->type->DATA_TYPE.type;
   else
      return checker_infer_expression_no_res(ckr, mod, node->expr);
}

type_t *checker_infer_alias(checker_t *ckr, module_t *mod, node_t *alias) {
   node_def(alias, ALIAS);

   return node->type->DATA_TYPE.type;
}
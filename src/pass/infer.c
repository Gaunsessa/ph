#include "checker.h"

type_t *checker_infer_expression(checker_t *ckr, module_t *mod, node_t *expr) {
   return type_get(ckr->type_handler, checker_infer_expression_no_res(ckr, mod, expr));
}

type_idx checker_infer_expression_no_res(checker_t *ckr, module_t *mod, node_t *expr) {
   switch (expr->type) {
      case NODE_NONE: return -1;
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
      case NODE_FUNCTION_DECLARATION: return expr->FUNCTION_DECLARATION.type->TYPE_IDX.type;
      case NODE_STRUCT: return checker_infer_expression_no_res(ckr, mod, expr->STRUCT.type);
      case NODE_TYPE_IDX: return expr->TYPE_IDX.type;
      case NODE_STRUCT_LITERAL: return checker_infer_expression_no_res(ckr, mod, expr->STRUCT_LITERAL.type);
      case NODE_ADDR_EXPRESSION: return checker_infer_addrexpr(ckr, mod, expr);
      case NODE_ALIAS: return checker_infer_alias(ckr, mod, expr);
      case NODE_SIGN_EXPRESSION: return checker_infer_expression_no_res(ckr, mod, expr->SIGN_EXPRESSION.expr);

      default: 
         print_node_type(expr->type);
         eprint("Invalid Expression!", expr->type);
   }
}

type_idx checker_infer_binexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   ERROR("UNIMPLEMENTED!");

   // node_def(expr, BINARY_EXPRESSION);

   // type_idx left  = checker_infer_expression_no_res(ckr, mod, node->left);
   // type_idx right = checker_infer_expression_no_res(ckr, mod, node->right);

   // switch (node->op) {
   //    case TOKEN_EQUALS_EQUALS:
   //    case TOKEN_NOT_EQUALS:
   //    case TOKEN_AND_AND:
   //    case TOKEN_OR_OR:
   //    case TOKEN_GREATER_THAN:
   //    case TOKEN_LESS_THAN:
   //    case TOKEN_GREATER_THAN_EQUALS:
   //    case TOKEN_LESS_THAN_EQUALS:
   //       return ht_get(BASE_TYPE_ENUM_VALUES, BASE_BOOL);
   //    default:
   //       if (left->type == TYPE_UNTYPED) {
   //          if (right->type == TYPE_UNTYPED)
   //             return (left->uninfer->base == BASE_F32 || right->uninfer->base == BASE_F32) ? BASE_UNTYPED_FLOAT : BASE_UNTYPED_INT;
   //          else return right;
   //       } else return left;
   // }
}

type_idx checker_infer_pathexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, PATH_EXPRESSION);

   if (node->expr->type == NODE_IDENTIFIER) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) return -1;

      type_idx type = checker_get_decl_both(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->IDENTIFIER.value);

      return type;
   } else if (node->expr->type == NODE_CALL_EXPRESSION) {
      if (!ht_exists_sv(ckr->modules, node->module->IDENTIFIER.value)) return -1;

      type_idx tidx = checker_get_decl(ht_get_sv(ckr->modules, node->module->IDENTIFIER.value), node->expr->CALL_EXPRESSION.func->IDENTIFIER.value, false);
      type_t *type  = type_get(ckr->type_handler, tidx);

      if (tidx < 0 || type->type != TYPE_FUNCTION) return -1;

      return type->ret;
   }

   return -1;
}

type_idx checker_infer_feildexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, FEILD_EXPRESSION);

   type_idx tidx = checker_resolve_base_type(mod, checker_infer_expression_no_res(ckr, mod, node->expr));
   if (tidx < 0) return -1;

   type_t *type = type_get(ckr->type_handler, tidx);

   if (type->type == TYPE_STRUCT)
      for (int i = 0; i < dy_len(type->feilds); i++)
         if (!wcscmp(dyi(type->feilds)[i].name, node->member)) 
            return dyi(type->feilds)[i].type;

   return -1;
}

type_idx checker_infer_methodexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, METHOD_EXPRESSION);

   type_idx tidx = checker_resolve_base_type(mod, checker_infer_expression_no_res(ckr, mod, node->expr));
   if (tidx < 0) return -1;

   type_t *type = type_get(ckr->type_handler, tidx);

   type_t *funct = NULL;

   if (type->type == TYPE_STRUCT)
      for (int i = 0; i < dy_len(type->funcs); i++)
         if (!wcscmp(dyi(type->funcs)[i].name, node->member))
            funct = type_get(ckr->type_handler, dyi(type->funcs)[i].type);

   if (funct == NULL || funct->type != TYPE_FUNCTION) return -1;

   return funct->ret;
}

type_idx checker_infer_addrexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, ADDR_EXPRESSION);

   type_idx tidx = checker_infer_expression_no_res(ckr, mod, node->expr);

   return type_init(ckr->type_handler, (type_t) { TYPE_PTR, .ptr_base = tidx });
}

type_idx checker_infer_derefexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, DEREF_EXPRESSION);

   type_t *type = type_get(ckr->type_handler, checker_infer_expression_no_res(ckr, mod, node->expr));

   return type != NULL && type->type == TYPE_PTR ? type->ptr_base : -1;
}

type_idx checker_infer_castexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CAST_EXPRESSION);

   return checker_infer_expression_no_res(ckr, mod, node->type);
}

type_idx checker_infer_callexpr(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *type = type_get(ckr->type_handler, checker_infer_identifier(ckr, mod, node->func));
   if (type == NULL) return -1;

   return type->ret;
}

type_idx checker_infer_callexpr_funct(checker_t *ckr, module_t *mod, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   return checker_infer_identifier(ckr, mod, node->func);
}

type_idx checker_infer_literal(checker_t *ckr, module_t *mod, node_t *lit) {
   ERROR("UNIMPLEMENTED!");
   // switch (lit->type) {
   //    case NODE_STRING_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_STRING);
   //    case NODE_FLOAT_LITERAL: return BASE_UNTYPED_FLOAT;
   //    case NODE_NUMBER_LITERAL: return BASE_UNTYPED_INT;
   //    default: eprint("Invalid Literal!");
   // }
}

type_idx checker_infer_identifier(checker_t *ckr, module_t *mod, node_t *ident) {
   node_def(ident, IDENTIFIER);

   type_idx tidx = checker_get_decl(mod, node->value, false);
   if (tidx < 0) return -1;

   return tidx;
}

type_idx checker_infer_var_decl(checker_t *ckr, module_t *mod, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   return checker_infer_expression_no_res(ckr, mod, node->type->type != NODE_NONE ? node->type : node->expr);
}

type_idx checker_infer_alias(checker_t *ckr, module_t *mod, node_t *alias) {
   ERROR("UNIMPLEMENTED!");
   // node_def(alias, ALIAS);

   // return node->type->DATA_TYPE.type;
}
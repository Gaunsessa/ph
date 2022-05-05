#include "checker.h"

type_t *checker_infer_expression(checker_t *ckr, node_t *expr) {
   return checker_reslove_type(ckr, checker_infer_expression_no_res(ckr, expr));
}

type_t *checker_infer_expression_no_res(checker_t *ckr, node_t *expr) {
   switch (expr->type) {
      case NODE_BINARY_EXPRESSION: return checker_infer_binexpr(ckr, expr);
      case NODE_CAST_EXPRESSION: return checker_infer_castexpr(ckr, expr);
      case NODE_DEREF_EXPRESSION: return checker_infer_derefexpr(ckr, expr);
      case NODE_CALL_EXPRESSION: return checker_infer_callexpr(ckr, expr);
      case NODE_ACCESS_EXPRESSION: return checker_infer_accessexpr(ckr, expr);
      case NODE_NUMBER_LITERAL:
      case NODE_FLOAT_LITERAL:
      case NODE_STRING_LITERAL: return checker_infer_literal(ckr, expr);
      case NODE_IDENTIFIER: return checker_infer_identifier(ckr, expr);
      case NODE_FUNCTION_DECLARATION: return expr->FUNCTION_DECLARATION.type->DATA_TYPE.type;
      case NODE_STRUCT: return checker_infer_expression_no_res(ckr, expr->STRUCT.type);
      case NODE_DATA_TYPE: return expr->DATA_TYPE.type;
      case NODE_ADDR_EXPRESSION: return checker_infer_addrexpr(ckr, expr);
      case NODE_ALIAS: return checker_infer_alias(ckr, expr);

      default: eprint("Invalid Expression!", expr->type);
   }
}

type_t *checker_infer_binexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, BINARY_EXPRESSION);

   type_t *left  = checker_infer_expression_no_res(ckr, node->left);
   type_t *right = checker_infer_expression_no_res(ckr, node->right);

   switch (node->op) {
      case TOKEN_EQUALS_EQUALS:
      case TOKEN_NOT_EQUALS:
      case TOKEN_AND_AND:
      case TOKEN_OR_OR:
      case TOKEN_GREATER_THAN:
      case TOKEN_LESS_THAN:
         return ht_get(BASE_TYPE_ENUM_VALUES, BASE_BOOL);
      default:
         if (left->type == TYPE_UNTYPED) {
            if (right->type == TYPE_UNTYPED)
               return (left->uninfer->base == BASE_F32 || right->uninfer->base == BASE_F32) ? BASE_UNTYPED_FLOAT : BASE_UNTYPED_INT;
            else return right;
         } else return left;
   }
}

type_t *checker_infer_accessexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, ACCESS_EXPRESSION);

   type_t *type = checker_reslove_base_type(ckr, checker_infer_expression_no_res(ckr, node->expr));
   if (type == NULL) return NULL;

   for (int i = 0; i < dy_len(type->feilds); i++)
      if (!strcmp(dyi(type->feilds)[i].name, node->member)) 
         return dyi(type->feilds)[i].type;

   return NULL;
}

type_t *checker_infer_addrexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, ADDR_EXPRESSION);

   type_t *type = checker_infer_expression_no_res(ckr, node->expr);

   return type_init((type_t) { TYPE_PTR, .ptr_base = type });
}

type_t *checker_infer_derefexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, DEREF_EXPRESSION);

   type_t *type = checker_infer_expression_no_res(ckr, node->expr);

   return type->type == TYPE_PTR ? type->ptr_base : NULL;
}

type_t *checker_infer_castexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, CAST_EXPRESSION);

   return node->type->DATA_TYPE.type;
}

type_t *checker_infer_callexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_reslove_type(ckr, checker_infer_identifier(ckr, node->func));
   if (ctype == NULL) return NULL;

   return ctype->ret;
}

type_t *checker_infer_callexpr_funct(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_reslove_type(ckr, checker_infer_identifier(ckr, node->func));
   if (ctype == NULL) return NULL;

   return ctype;
}

type_t *checker_infer_literal(checker_t *ckr, node_t *lit) {
   switch (lit->type) {
      case NODE_STRING_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_STRING);
      case NODE_FLOAT_LITERAL: return BASE_UNTYPED_FLOAT;
      case NODE_NUMBER_LITERAL: return BASE_UNTYPED_INT;
      default: eprint("Invalid Literal!");
   }
}

type_t *checker_infer_identifier(checker_t *ckr, node_t *ident) {
   node_def(ident, IDENTIFIER);

   decl_t *decl = checker_get_decl(ckr, node->value);
   if (decl == NULL || decl->is_typedef) return NULL;

   return decl->type;
}

type_t *checker_infer_var_decl(checker_t *ckr, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   if (node->type->DATA_TYPE.type->type != TYPE_INFER)
      return node->type->DATA_TYPE.type;
   else
      return checker_infer_expression_no_res(ckr, node->expr);
}

type_t *checker_infer_alias(checker_t *ckr, node_t *alias) {
   node_def(alias, ALIAS);

   return node->type->DATA_TYPE.type;
}
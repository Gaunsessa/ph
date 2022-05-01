#include "checker.h"

type_t *checker_infer_expression(checker_t *ckr, node_t *expr) {
   switch (expr->type) {
      case NODE_BINARY_EXPRESSION: return checker_infer_binexpr(ckr, expr);
      case NODE_UNARY_EXPRESSION: return checker_infer_uryexpr(ckr, expr);
      case NODE_CALL_EXPRESSION: return checker_infer_callexpr(ckr, expr);
      case NODE_NUMBER_LITERAL:
      case NODE_STRING_LITERAL: return checker_infer_literal(ckr, expr);
      case NODE_IDENTIFIER: return checker_infer_identifier(ckr, expr);
      case NODE_FUNCTION_DECLARATION: return expr->FUNCTION_DECLARATION.type->DATA_TYPE.type;
      case NODE_ALIAS: return checker_infer_alias(ckr, expr);

      default: eprint("Invalid Expression!", expr->type);
   }
}

type_t *checker_infer_binexpr(checker_t *ckr, node_t *expr) {
   // node_def(expr, BINARY_EXPRESSION);

   eprint("AAAA");  
}

type_t *checker_infer_uryexpr(checker_t *ckr, node_t *expr) {
   eprint("URYEXPR UNIMPLMENTED!");
}

type_t *checker_infer_callexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_infer_identifier(ckr, node->func);
   if (ctype == NULL) return NULL;

   return ctype->ret;
}

type_t *checker_infer_callexpr_funct(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *ctype = checker_infer_identifier(ckr, node->func);
   if (ctype == NULL) return NULL;

   return ctype;
}

type_t *checker_infer_literal(checker_t *ckr, node_t *lit) {
   switch (lit->type) {
      case NODE_STRING_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_STRING);
      case NODE_NUMBER_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_INT);
      default: eprint("Invalid Literal!");
   }
}

type_t *checker_infer_identifier(checker_t *ckr, node_t *ident) {
   node_def(ident, IDENTIFIER);

   return type_deref_ref(checker_reslove_type(ckr, checker_get_type(ckr, node->value)));
}

type_t *checker_infer_var_decl(checker_t *ckr, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   if (node->type->DATA_TYPE.type->type != TYPE_INFER)
      return node->type->DATA_TYPE.type;
   else
      return checker_infer_expression(ckr, node->expr);
}

type_t *checker_infer_alias(checker_t *ckr, node_t *alias) {
   node_def(alias, ALIAS);

   return type_init((type_t) { TYPE_TYPE_REF, .ref = checker_reslove_type(ckr, node->type->DATA_TYPE.type) });
}
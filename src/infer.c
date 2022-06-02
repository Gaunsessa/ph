#include "infer.h"

type_idx infer_expression(sym_table_t *tbl, sym_module_t *mod, size_t scope, node_t *expr) {
   switch (expr->type) {
      case NODE_IDENTIFIER: return sym_table_get_both(mod, expr->IDENTIFIER.value, scope);
      case NODE_TYPE_IDX: return expr->TYPE_IDX.type;
      case NODE_PATH_EXPRESSION: return sym_table_get_both(ht_get_sv(tbl->modules, expr->PATH_EXPRESSION.module->IDENTIFIER.value), expr->PATH_EXPRESSION.expr->IDENTIFIER.value, 0);
      case NODE_STRING_LITERAL: return BASE_STRING;
      case NODE_NUMBER_LITERAL: return BASE_INT;
      case NODE_FLOAT_LITERAL: return BASE_F32;
      case NODE_STRUCT_LITERAL: return infer_expression(tbl, mod, scope, expr->STRUCT_LITERAL.type);
      case NODE_FUNCTION_DECLARATION: return infer_expression(tbl, mod, scope, expr->FUNCTION_DECLARATION.type);
      case NODE_STRUCT: return expr->STRUCT.type->TYPE_IDX.type;

      default: 
         print_node_type(expr->type);
         eprint("Unhandled infer expression: ", expr->type);
   }
}
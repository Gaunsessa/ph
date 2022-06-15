#include "infer.h"

type_idx infer_expression(sym_table_t *tbl, sym_module_t *mod, size_t scope, node_t *expr) {
   switch (expr->type) {
      case NODE_STRING_LITERAL: return BASE_STRING;
      case NODE_NUMBER_LITERAL: return BASE_INT;
      case NODE_FLOAT_LITERAL: return BASE_F32;
      case NODE_BOOL_LITERAL: return BASE_BOOL;

      case NODE_IDENTIFIER: {
         node_def(expr, IDENTIFIER);

         return sym_table_get_both(mod, node->value, scope);
      } break;
      case NODE_TYPE_IDX: {
         node_def(expr, TYPE_IDX);

         return node->type;
      } break;
      case NODE_PATH_EXPRESSION: {
         node_def(expr, PATH_EXPRESSION);

         if (node->expr->type == NODE_CALL_EXPRESSION) {
            type_t *type  = type_get(tbl->ty_hdl, sym_table_get_both(sym_table_get_module(tbl, node->module->IDENTIFIER.value), node->expr->CALL_EXPRESSION.func->IDENTIFIER.value, 0));

            return type != NULL && type->type == TYPE_FUNCTION ? type->ret : -1;
         } else return sym_table_get_both(sym_table_get_module(tbl, node->module->IDENTIFIER.value), node->expr->IDENTIFIER.value, 0);
      } break;
      case NODE_STRUCT_LITERAL: {
         node_def(expr, STRUCT_LITERAL);

         return infer_expression(tbl, mod, scope, node->type);
      } break;
      case NODE_FUNCTION_DECLARATION: {
         node_def(expr, FUNCTION_DECLARATION);

         return infer_expression(tbl, mod, scope, node->type);
      } break;
      case NODE_STRUCT: {
         node_def(expr, STRUCT);

         return node->type->TYPE_IDX.type;
      } break;
      case NODE_CALL_EXPRESSION: {
         node_def(expr, CALL_EXPRESSION);

         type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->func));

         return type == NULL ? -1 : type->ret;
      } break;
      case NODE_ALIAS: {
         node_def(expr, ALIAS);

         // type_t *type = type_get(tbl->ty_hdl, node->type->TYPE_IDX.type);

         // return sym_table_get(mod, type->name, scope, true);
         return node->type->TYPE_IDX.type;
      } break;
      case NODE_CAST_EXPRESSION: {
         node_def(expr, CAST_EXPRESSION);

         return infer_expression(tbl, mod, scope, node->type);
      } break;
      case NODE_FEILD_EXPRESSION: {
         node_def(expr, FEILD_EXPRESSION);

         type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->expr));
         if (type == NULL) return -1;

         if (type->type == TYPE_PTR)
            type = type_get(tbl->ty_hdl, type->ptr_base);

         if (type == NULL || type->type != TYPE_STRUCT) return -1;

         for (int i = 0; i < dy_len(type->feilds); i++)
            if (!wcscmp(dyi(type->feilds)[i].name, node->member))
               return dyi(type->feilds)[i].type;

         return -1;
      } break;

      default: 
         print_node_type(expr->type);
         eprint("Unhandled infer expression: ", expr->type);
   }
}
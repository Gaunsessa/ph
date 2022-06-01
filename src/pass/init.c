#include "init.h"

void init_pass(node_t *AST, sym_table_t *tbl) {
   node_walk(AST, tbl, 0, init_special, init_start, init_end);
}

bool init_special(node_t *node) {
   return false;
}

void init_start(node_t *node, sym_table_t *tbl, size_t scope) {
   if (scope != 0) return;

   switch (node->type) {
      case NODE_VARIABLE_DECLARATION:
         if (node->VARIABLE_DECLARATION.expr->type == NODE_FUNCTION_DECLARATION) {
            sym_table_set(tbl, node->VARIABLE_DECLARATION.ident->IDENTIFIER.value, scope, false, 69);
         }

         break;
      default: break;
   }
}

void init_end(node_t *node, sym_table_t *tbl, size_t scope) {

}
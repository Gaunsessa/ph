#include "globals.h"

void globals_pass(node_t *AST, sym_table_t *tbl) {
   node_walk(AST, tbl, 0, globals_special, globals_start, globals_end);
}

bool globals_special(node_t *node) { 
   return false; 
}

void globals_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   switch (node->type) {
      case NODE_VARIABLE_DECLARATION:
         sym_table_set(mod, node->VARIABLE_DECLARATION.ident->IDENTIFIER.value, scope, false, infer_expression(tbl, mod, scope, node->VARIABLE_DECLARATION.expr));
      default: return;
   }
}

void globals_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   return;
}
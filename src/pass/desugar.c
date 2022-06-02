// #include "desugar.h"

// void desugar_pass(node_t *AST, sym_table_t *tbl) {
//    node_walk(AST, tbl, 0, desugar_special, desugar_start, desugar_end);
// }

// bool desugar_special(node_t *node) {
//    return false;
// }

// void desugar_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
//    switch (node->type) {
//       case NODE_DATA_TYPE:;
//          type_idx tidx = -1;

//          switch (node->DATA_TYPE.type->type) {
//             case NODE_IDENTIFIER:
//             case NODE_PATH_EXPRESSION:
//             case NODE_TYPE_IDX: return;
//             case NODE_PTR_TYPE:
               
//          }

//          node->DATA_TYPE.type->type = NODE_TYPE_IDX;
//          node->DATA_TYPE.type->TYPE_IDX.type = tidx;

//          break;
//       default: return;
//    }
// }

// void desugar_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
//    return;
// }
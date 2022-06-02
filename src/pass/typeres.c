#include "typeres.h"

void typeres_resolve(node_t *AST) {
   node_walker(AST, typeres_special, typeres_start, typeres_end);
}

bool typeres_special(node_t *node) {
   return false;
}

void typeres_start(node_t *node) {
   switch (node->type) {
      // case NODE_DATA_TYPE: return typeres_data_type(node);

      default: return;
   }
}

void typeres_end(node_t *node) {

}

void typeres_data_type(node_t *type) {
   // node_def(type, DATA_TYPE);

   // if (node->type->type == NODE_FUNCTION_TYPE) {
   //    type_t type = {
   //       .type = TYPE_FUNCTION,
   //       .args = dy_init(struct { wchar_t *name; type_idx type; }),
   //    };
   
   //    for (int i = 0; i < dy_len(node->type->FUNCTION_TYPE.arg_names); i++) {
   //       struct { wchar_t *name; type_idx type; } arg = { 
   //          dyi(node->type->FUNCTION_TYPE.arg_names)[i], 
   //          checker_infer_expression_no_res(ckr, mod, dyi(node->type->FUNCTION_TYPE.arg_types)[i]) 
   //       };
   
   //       dy_push_unsafe(type.args, arg);
   //    }
   
   //    node_replace(node->type, (node_t) {
   //       .type = NODE_TYPE_IDX,
   //       .TYPE_IDX = { type_init(ckr->type_handler, type) }
   //    });
   // }
}
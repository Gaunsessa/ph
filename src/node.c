#include "node.h"

#include "prims.h"

#define _walk_arr(arr, walker) ({ for (int i = 0; i < dy_len(arr); i++) node_walker(dyi(arr)[i], walker); })

void node_walker(node_t *node, void (*walker)(node_t *node)) {
   walker(node);

   switch (node->type) {
      case NODE_FILE:
         _walk_arr(node->FILE.stmts, walker);
         break;
      case NODE_BLOCK:
         _walk_arr(node->BLOCK.stmts, walker);
         break;
      case NODE_BINARY_EXPRESSION:
         node_walker(node->BINARY_EXPRESSION.left, walker);
         node_walker(node->BINARY_EXPRESSION.right, walker);
         break;
      case NODE_UNARY_EXPRESSION:
         node_walker(node->UNARY_EXPRESSION.ident, walker);
         break;
      case NODE_CALL_EXPRESSION:
         node_walker(node->CALL_EXPRESSION.func, walker);
         _walk_arr(node->CALL_EXPRESSION.args, walker);
         break;
      case NODE_VARIABLE_DECLARATION:
         node_walker(node->VARIABLE_DECLARATION.expr, walker);
         node_walker(node->VARIABLE_DECLARATION.ident, walker);
         node_walker(node->VARIABLE_DECLARATION.type, walker);
         break;
      case NODE_FUNCTION_DECLARATION:
         node_walker(node->FUNCTION_DECLARATION.stmt, walker);
         node_walker(node->FUNCTION_DECLARATION.type, walker);
         break;
      case NODE_IF:
         node_walker(node->IF.falsecase, walker);
         node_walker(node->IF.truecase, walker);
         node_walker(node->IF.cond, walker);
         break;
      case NODE_FOR:
         node_walker(node->FOR.cond, walker);
         node_walker(node->FOR.init, walker);
         node_walker(node->FOR.post, walker);
         node_walker(node->FOR.stmt, walker);
         break;
      case NODE_RETURN:
         node_walker(node->RETURN.value, walker);
         break;
      default: break;
   }
}

void node_free(node_t *node) {
   switch (node->type) {
#define _NODE(type, name, ident, i, ...) _freetype(&node->ident.name, primtype(node->ident.name));
#define NODE(ident, ...)                    \
      case NODE_##ident:                    \
         M_MAP2(_NODE, ident, __VA_ARGS__); \
         break;                             \

      NODE_TYPES
#undef NODE
#undef _NODE
      default: return;
   }
}

void print_node(node_t *node) {
   switch (node->type) {
#define _NODE(type, name, ident, i, ...) { _printtype(&node->ident.name, primtype(node->ident.name)); printf(" "); }
#define NODE(ident, ...)                         \
      case NODE_##ident: {                       \
         if (node->type == NODE_EMPTY) return;   \
         printf("%s : ", M_STR(NODE_##ident));   \
         M_MAP2(_NODE, ident, __VA_ARGS__);      \
         printf("");                             \
      } break;                                   \

      NODE_TYPES
#undef NODE
#undef _NODE

      default: eprint("Error: unknown node type!");
   }
}

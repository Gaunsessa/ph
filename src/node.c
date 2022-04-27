#include "node.h"

#include "prims.h"

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

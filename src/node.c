#include "node.h"

#include "prims.h"

#define _walk_arr(arr) ({ for (int i = 0; i < dy_len(arr); i++) node_walker(dyi(arr)[i], start, end); })

void node_walker(node_t *node, void (*start)(node_t *node), void (*end)(node_t *node)) {
   start(node);

   // Special Case
   switch (node->type) {
      case NODE_VARIABLE_DECLARATION:
         node_walker(node->VARIABLE_DECLARATION.expr, start, end);
         node_walker(node->VARIABLE_DECLARATION.type, start, end);

         end(node);

         return;
      case NODE_FUNCTION_DECLARATION:
         node_walker(node->FUNCTION_DECLARATION.stmt, start, end);
         end(node);

         return;
      default: break;
   }

#define _NODE(a, b, c, i, ...)                                                                                    \
   ({                                                                                                             \
      if (__builtin_types_compatible_p(a, struct node_t *)) node_walker((void *)*(void **)&node->c.b, start, end);           \
      else if(__builtin_types_compatible_p(a, dynarr_t(struct node_t *))) _walk_arr((void ***)*(void **)&node->c.b);        \
   });                                                                                                            \

#define NODE(ident, ...) case NODE_##ident: { M_MAP2(_NODE, ident, __VA_ARGS__) } break;
   switch (node->type) {
      case _NODE_: break;
      NODE_TYPES
   }
#undef NODE
#undef _NODE

   end(node);
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

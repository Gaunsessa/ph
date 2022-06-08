#include "node.h"

#include "prims.h"

#define _walk_arr(arr) ({ for (int i = 0; i < dy_len(arr); i++) node_walker(dyi(arr)[i], special, start, end); })
#define node_walk_arr(arr, tbl, mod, scope, hscope, special, start, end) ({ for (int i = 0; i < dy_len(arr); i++) _node_walk(dyi(arr)[i], tbl, mod, scope, hscope, special, start, end); })

void node_walker(node_t *node, bool (*special)(node_t *node), void (*start)(node_t *node), void (*end)(node_t *node)) {
   start(node);

   if (special(node)) {
      end(node);

      return;
   }

#define _NODE(a, b, c, i, ...)                                                                                    \
   ({                                                                                                             \
      if (__builtin_types_compatible_p(a, struct node_t *)) node_walker((void *)*(void **)&node->c.b, special, start, end);           \
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

void node_walk(node_t *node, sym_table_t *tbl, size_t scope, bool (*special)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope), void (*start)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope), void (*end)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope)) {
   size_t hscope = scope;

   _node_walk(node, tbl, NULL, scope, &hscope, special, start, end);
}

void _node_walk(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope, bool (*special)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope), void (*start)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope), void (*end)(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope)) {
   start(node, tbl, mod, scope);

   size_t *ohscope = hscope;
   if (node->type == NODE_FILE) {
      if (!ht_exists_sv(tbl->modules, node->FILE.name))
         sym_table_push_module(tbl, node->FILE.name);

      mod = sym_table_get_module(tbl, node->FILE.name);

      size_t *nhscope = calloc(1, sizeof(size_t));
      hscope = nhscope;
   }

   if (mod != NULL && M_COMPARE(
      node->type, 
      NODE_BLOCK, NODE_IMPL, 
      NODE_IF,    NODE_FOR
   )) {
      (*hscope)++;

      if (!ht_exists(mod->sinf, *hscope))
         sym_table_push_scope(mod, *hscope, scope);

      scope = *hscope;
   }

   if (special(node, tbl, mod, scope, hscope)) goto END;

#define _NODE(a, b, c, i, ...)                                                                        \
   ({                                                                                                 \
      if (__builtin_types_compatible_p(a, struct node_t *))                                           \
         _node_walk((void *)*(void **)&node->c.b, tbl, mod, scope, hscope, special, start, end);      \
      else if(__builtin_types_compatible_p(a, dynarr_t(struct node_t *)))                             \
         node_walk_arr((void ***)*(void **)&node->c.b, tbl, mod, scope, hscope, special, start, end); \
   });                                                                                                \

#define NODE(ident, ...) case NODE_##ident: { M_MAP2(_NODE, ident, __VA_ARGS__) } break;
   switch (node->type) {
      case _NODE_: break;
      NODE_TYPES
   }
#undef NODE
#undef _NODE

END:
   if (mod != NULL && M_COMPARE(
      node->type, 
      NODE_BLOCK, NODE_IMPL, 
      NODE_IF,    NODE_FOR
   )) scope = ht_get(mod->sinf, scope)->parent->id;

   if (node->type == NODE_FILE) {
      free(hscope);

      hscope = ohscope;
   }

   end(node, tbl, mod, scope);
}

void node_replace(node_t *onode, node_t nnode) {
   switch (onode->type) {
#define _NODE(type, name, ident, i, ...) _freetype(&onode->ident.name, primtype(onode->ident.name));
#define NODE(ident, ...)                    \
      case NODE_##ident:                    \
         M_MAP2(_NODE, ident, __VA_ARGS__); \
         break;                             \

      NODE_TYPES
#undef NODE
#undef _NODE
      default: break;
   }

   memcpy(onode, &nnode, sizeof(node_t));
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

void _print_tabs(int amt) {
   for (int i = 0; i < amt; i++)
      printf(" ");
}

void print_node(node_t *node) {
   static int t = 0;

   switch (node->type) {
#define _NODE(type, name, ident, i, ...) { _print_tabs(t); _printtype(&node->ident.name, primtype(node->ident.name)); printf(M_VALEN(__VA_ARGS__) ? "\n" : ""); }
#define NODE(ident, ...)                         \
      case NODE_##ident: {                       \
         if (node->type == NODE_EMPTY) return;   \
         printf("%s : ", M_STR(NODE_##ident));   \
         printf("\n");                           \
         t += 2;                                 \
         M_MAP2(_NODE, ident, __VA_ARGS__);      \
         t -= 2;                                 \
      } break;                                   \

      NODE_TYPES
#undef NODE
#undef _NODE

      default: eprint("Error: unknown node type!");
   }
}

void print_node_type(NODE_TYPE type) {
   switch (type) {
#define NODE(ident, ...) case NODE_##ident: printf("%s\n", M_STR(NODE_##ident)); break;
      NODE_TYPES
#undef NODE

      default: return;
   }
}

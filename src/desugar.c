#include "desugar.h"

void _desugar_start(node_t *node) {
   desugar_start(node, NULL);
}

void _desugar_end(node_t *node) {
   desugar_end(node, NULL);
}

void desugar_desugar(node_t *AST) {
   desugar_t des = { NULL, 0, dy_init(node_t *), NULL };

   desugar_start(NULL, &des);
   desugar_end(NULL, &des);

   node_walker(AST, desugar_special, _desugar_start, _desugar_end);

   dy_free(des.parent_scopes);
}

bool desugar_special(node_t *node) {
   return false;
}

void desugar_start(node_t *node, desugar_t *des) {
   static desugar_t *desg = NULL;
   if (des != NULL) desg = des;
   else {
      switch (node->type) {
         case NODE_FILE: desugar_file(node, desg); break;
         case NODE_IDENTIFIER: desugar_ident(node, desg); break;
         case NODE_DATA_TYPE: desugar_data_type(node, desg); break;
         default: break;
      }

      switch (node->type) {
         case NODE_FILE:
         case NODE_FUNCTION_DECLARATION:
         case NODE_IMPL:
            dy_push(desg->parent_scopes, node);
            desg->parent_scope = node;
            break;
         default: break;
      }
   }
}

void desugar_end(node_t *node, desugar_t *des) {
   static desugar_t *desg = NULL;
   if (des != NULL) desg = des;
   else {
      switch (node->type) {
         default: break;
      }

      switch (node->type) {
         case NODE_FILE:
         case NODE_FUNCTION_DECLARATION:
         case NODE_IMPL:
            if (dy_len(desg->parent_scopes) > 0) {
               dy_pop(desg->parent_scopes);
               desg->parent_scope = dy_len(desg->parent_scopes) ? 
                  dyi(desg->parent_scopes)[dy_len(desg->parent_scopes) - 1] : NULL;
            } else desg->parent_scope = NULL;
            break;
         default: break;
      }
   }
}

void desugar_file(node_t *file, desugar_t *des) {
   node_def(file, FILE);

   des->module_name = node->name;
   des->module_name_len = wcslen(node->name);
}

void desugar_ident(node_t *ident, desugar_t *des) {
   node_def(ident, IDENTIFIER);

   if (des->parent_scope->type == NODE_FILE) {
      // size_t ident_len = wcslen(node->value);

      // node->value = realloc(node->value, ident_len * sizeof(wchar_t) + sizeof(L"_") - 1 + 1);
      // node->value[ident_len] = L'_';
      // node->value[ident_len + 1] = 0;

      node->value = desugar_rename_ident(des, node->value);
   }
}

void desugar_data_type(node_t *dtype, desugar_t *des) {
   node_def(dtype, DATA_TYPE);

   desugar_type(node->type, des);
}

void desugar_type(type_t *type, desugar_t *des) {
   type = desugar_resolve_type(type);

   if (type->type == TYPE_ALIAS) {
      type->name = desugar_rename_ident(des, type->name);
   } else if (type->type == TYPE_FUNCTION) {
      for (int i = 0; i < dy_len(type->args); i++)
         desugar_type(dyi(type->args)[i].type, des);

      desugar_type(type->ret, des);
   }
}

// TODO: so many duplicate wcslens
wchar_t *_desugar_rename_ident(desugar_t *des, wchar_t *ident, size_t amt, ...) {
   va_list args;
   va_start(args, amt);

   size_t len = des->module_name_len + wcslen(ident) + 2;

   for (int i = 0; i < amt; i++)
      len += wcslen(va_arg(args, wchar_t *)) + 1;

   ident = realloc(ident, len * sizeof(wchar_t));
   // wcscpy(ident + len - wcslen(ident) - 1, ident);
   memmove(ident + len - wcslen(ident) - 1, ident, (wcslen(ident) + 1) * sizeof(wchar_t));

   va_start(args, amt);

   wcscpy(ident, des->module_name);
   ident[des->module_name_len] = L'_';

   int pos = des->module_name_len + 1;
   for (int i = 0; i < amt; i++) {
      wchar_t *str = va_arg(args, wchar_t *);
      size_t len   = wcslen(str);

      memcpy(ident + pos, str, len * sizeof(wchar_t));

      pos += len + 1;
      ident[pos - 1] = L'_';
   }

   va_end(args);

   return ident;
}

type_t *desugar_resolve_type(type_t *type) {
   if (type == NULL) return NULL;

   switch (type->type) {
      case TYPE_PTR: return desugar_resolve_type(type->ptr_base);
      case TYPE_ARRAY: return desugar_resolve_type(type->arr_base);
      case TYPE_UNTYPED: return desugar_resolve_type(type->uninfer);

      default: return type;
   }
}
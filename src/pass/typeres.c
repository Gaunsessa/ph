#include "typeres.h"

void typeres_pass(node_t *AST, sym_table_t *tbl) {
   node_walk(AST, tbl, 0, typeres_special, typeres_start, typeres_end);
}

bool typeres_special(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope, size_t *hscope) {
   switch (node->type) {
      // case NODE_VARIABLE_DECLARATION: 
         // _node_walk(node->VARIABLE_DECLARATION.expr, tbl, mod, scope, hscope, typeres_special, typeres_start, typeres_end);
         // return true;
      default: return false;
   }
}

void typeres_start(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   // if (scope == 0) return;

   switch (node->type) {
      case NODE_VARIABLE_DECLARATION: return typeres_vardecl(node, tbl, mod, scope);
      case NODE_FUNCTION_DECLARATION: return typeres_funcdecl(node, tbl, mod, scope);
      default: return;
   }
}

void typeres_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   return;
}

void typeres_vardecl(node_t *vdecl, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(vdecl, VARIABLE_DECLARATION);

   type_idx tidx = typeres_resolve_type(
      infer_expression(tbl, mod, scope, node->type->type != NODE_NONE ? node->type : node->expr),
      tbl, mod, scope
   );
   type_t *type  = type_get(tbl->ty_hdl, tidx);

   sym_table_set(
      mod, 
      node->ident->IDENTIFIER.value, 
      scope, 
      M_COMPARE(node->expr->type, NODE_STRUCT, NODE_ALIAS), 
      tidx
   );

   // node->type->TYPE_IDX.type = tidx;

   if (type != NULL && M_COMPARE(node->expr->type, NODE_STRUCT, NODE_ALIAS)) {
      type->name   = wcsdup(node->ident->IDENTIFIER.value);
      type->module = wcsdup(mod->name);
      // type->name = L"ALIAS";

      if (node->expr->type == NODE_ALIAS) type->type = TYPE_ALIAS;
   }
}

void typeres_funcdecl(node_t *fdecl, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(fdecl, FUNCTION_DECLARATION);

   type_t *ftype = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, node->type));
   if (ftype == NULL) eprint("Cannot infer type!");

   for (int i = 0; i < dy_len(ftype->args); i++)
      sym_table_set(mod, dyi(ftype->args)[i].name, scope, false, dyi(ftype->args)[i].type);

   if (ftype->self.name != NULL)
      sym_table_set(mod, ftype->self.name, scope, false, ftype->self.type);
}

type_idx typeres_resolve_type(type_idx idx, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   type_t *type = type_get(tbl->ty_hdl, idx);
   if (type == NULL) return -1;

   switch (type->type) {
      case TYPE_INFER: eprint("Cannot resolve type of infer!");
      case TYPE_NONE: 
         if (!ht_exists_sv(tbl->modules, type->module)) return -1;

         return sym_table_get_both(sym_table_get_module(tbl, type->module), type->name, scope);
      case TYPE_PTR: 
         type->ptr_base = typeres_resolve_type(type->ptr_base, tbl, mod, scope); 
         break;
      case TYPE_ARRAY:
         type->arr_base = typeres_resolve_type(type->arr_base, tbl, mod, scope);
         break;
      default: break;
   }

   return idx;
}

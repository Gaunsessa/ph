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
      case NODE_TYPE_BASE: return typeres_typebase(node, tbl, mod, scope);
      case NODE_VARIABLE_DECLARATION: return typeres_vardecl(node, tbl, mod, scope);
      case NODE_FUNCTION_DECLARATION: return typeres_funcdecl(node, tbl, mod, scope);
      case NODE_IMPL: return typeres_impl(node, tbl, mod, scope);
      default: return;
   }
}

void typeres_end(node_t *node, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   return;
}

void typeres_typebase(node_t *btyp, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   // node_def(btyp, TYPE_BASE);

   // type_t *type = type_get(tbl->ty_hdl, infer_expression(tbl, mod, scope, btyp));
   // if (type == NULL) eprint("Failed to infer type!?!?!");

   // switch (node->type->type) {
   //    case NODE_TYPE_FUNCTION: {
   //       node_def(btyp->TYPE_BASE.type, TYPE_FUNCTION);

   //       for (int i = 0; i < dy_len(type->args); i++)
   //          if (dyi(type->args)[i].type < 0)
   //             dyi(type->args)[i].type = infer_expression(tbl, mod, scope, dyi(node->arg_types)[i]);

   //       if (type->ret < 0) type->ret = infer_expression(tbl, mod, scope, node->ret);
   //    } break;
   // }
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

   // ftype->ret = infer_expression(tbl, mod, scope, node->type->TYPE_BASE.type->TYPE_FUNCTION.ret);
}

void typeres_impl(node_t *impl, sym_table_t *tbl, sym_module_t *mod, size_t scope) {
   node_def(impl, IMPL);

   type_idx tidx = infer_expression(tbl, mod, scope, node->type);
   type_t *type  = type_get(tbl->ty_hdl, tidx);
   if (type == NULL) eprint("Cannot infer type!");

   if (type->type != TYPE_STRUCT) return;

   for (int i = 0; i < dy_len(node->funcs); i++) {
      node_t *func = dyi(node->funcs)[i];

      struct { wchar_t *name; type_idx type; } fun;

      fun.name = wcsdup(func->VARIABLE_DECLARATION.ident->IDENTIFIER.value);
      fun.type = infer_expression(tbl, mod, scope, func->VARIABLE_DECLARATION.expr);

      dy_push_unsafe(type->funcs, fun);
   }
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

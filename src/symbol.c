#include "symbol.h"

sym_table_t *sym_table_new() {
   sym_table_t *tbl = malloc(sizeof(sym_table_t));

   tbl->ty_hdl  = type_handler_new();
   tbl->modules = ht_init_sv(wchar_t, sym_module_t *);

   return tbl;
}

void sym_table_push_module(sym_table_t *tbl, wchar_t *name) {
   sym_module_t *mod = malloc(sizeof(sym_module_t));

   mod->name = wcsdup(name);

   mod->decls = ht_init_sv(wchar_t, sym_entry_t *);
   mod->types = ht_init_sv(wchar_t, sym_entry_t *);

   mod->sinf     = ht_init(int, struct sinf_t *);
   mod->sinf_arr = (struct sinf_t) {
      0,
      NULL,
      dy_init(struct sinf_t *),
   };

   ht_set(mod->sinf, 0, &mod->sinf_arr);

   ht_set_sv(tbl->modules, name, mod);
}

void sym_table_push_scope(sym_module_t *mod, size_t scope, size_t parent) {
   struct sinf_t *sinf = malloc(sizeof(struct sinf_t));

   sinf->id       = scope;
   sinf->parent   = ht_get(mod->sinf, parent);
   sinf->children = dy_init(struct sinf_t *);

   dy_push(sinf->parent->children, sinf);
   ht_set(mod->sinf, scope, sinf);
}

void sym_table_set(sym_module_t *mod, wchar_t *name, size_t scope, bool typedf, type_idx type) {
   ht_t(wchar_t, sym_entry_t *) ht = typedf ? mod->types : (void *)mod->decls;

   sym_entry_t *ent = malloc(sizeof(sym_entry_t));

   ent->name  = wcsdup(name);
   ent->scope = scope;
   ent->type  = type;
   ent->next  = ht_exists_sv(ht, name) ? ht_get_sv(ht, name) : NULL;

   ht_set_sv(ht, name, ent);
}

type_idx sym_table_get_cur(sym_module_t *mod, wchar_t *name, size_t scope, bool typedf) {
   if (mod == NULL) return -1;

   ht_t(wchar_t *, sym_entry_t *) ht = typedf ? mod->types : (void *)mod->decls;
   if (!ht_exists_sv(ht, name)) return -1;

   sym_entry_t *entry = ht_get_sv(ht, name);

   while (entry != NULL && entry->scope != scope) entry = entry->next;

   return entry != NULL ? entry->type : -1;
}

type_idx sym_table_get(sym_module_t *mod, wchar_t *name, size_t scope, bool typedf) {
   if (mod == NULL) return -1;

   ht_t(wchar_t *, sym_entry_t *) ht = typedf ? mod->types : (void *)mod->decls;
   if (!ht_exists_sv(ht, name)) return -1;

   sym_entry_t *fentry = ht_get_sv(ht, name); 

   sym_entry_t *entry = fentry;
   sym_entry_t *csco  = NULL;
   do if (entry->scope == scope) 
      csco = entry;
   while ((entry = entry->next) != NULL);

   if (csco == NULL) {
      if (!ht_exists(mod->sinf, scope)) eprint("Invalid scope passed in!");

      struct sinf_t *pscope = ht_get(mod->sinf, scope)->parent;
      
      return pscope == NULL ? -1 : sym_table_get(mod, name, pscope->id, typedf);
   } else return csco->type;
}

// TODO: this should probably return the one with the closest scope
type_idx sym_table_get_cur_both(sym_module_t *mod, wchar_t *name, size_t scope) {
   type_idx ret = sym_table_get_cur(mod, name, scope, false);
   if (ret < 0) ret = sym_table_get_cur(mod, name, scope, true);

   return ret;
}

type_idx sym_table_get_both(sym_module_t *mod, wchar_t *name, size_t scope) {
   type_idx ret = sym_table_get(mod, name, scope, false);
   if (ret < 0) ret = sym_table_get(mod, name, scope, true);

   return ret;
}

sym_module_t *sym_table_get_module(sym_table_t *tbl, wchar_t *name) {
   return name == NULL || !ht_exists_sv(tbl->modules, name) ? NULL : ht_get_sv(tbl->modules, name);
}

void _sym_entry_free(void *ent) {
   sym_entry_t *entry = ent;

   free(entry->name);

   if (entry->next != NULL) 
      _sym_entry_free(entry->next);

   free(entry);
}

void _sym_module_free(void *mod) {
   sym_module_t *module = mod;

   free(module->name);

   ht_free_func(module->decls, _sym_entry_free);
   ht_free_func(module->types, _sym_entry_free);

   free(module);
}

void sym_table_free(sym_table_t *tbl) {
   ht_free_func(tbl->modules, _sym_module_free);

   type_handler_free(tbl->ty_hdl);
   free(tbl);
}
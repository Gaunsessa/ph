#include "symbol.h"

sym_table_t *sym_table_new() {
   sym_table_t *tbl = malloc(sizeof(sym_table_t));

   tbl->decls = ht_init_sv(wchar_t, sym_entry_t *);
   tbl->types = ht_init_sv(wchar_t, sym_entry_t *);

   tbl->sinf     = ht_init(int, struct sinf_t *);
   tbl->sinf_arr = (struct sinf_t) {
      0,
      NULL,
      dy_init(struct sinf_t *),
   };

   ht_set(tbl->sinf, 0, &tbl->sinf_arr);

   return tbl;
}

void sym_table_push_scope(sym_table_t *tbl, size_t scope, size_t parent) {
   struct sinf_t *sinf = malloc(sizeof(struct sinf_t));

   sinf->id       = scope;
   sinf->parent   = ht_get(tbl->sinf, parent);
   sinf->children = dy_init(struct sinf_t *);

   dy_push(sinf->parent->children, sinf);
   ht_set(tbl->sinf, scope, sinf);
}

void sym_table_set(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf, type_idx type) {
   ht_t(wchar_t, sym_entry_t *) ht = typedf ? tbl->types : (void *)tbl->decls;

   sym_entry_t *ent = malloc(sizeof(sym_entry_t));

   ent->name  = wcsdup(name);
   ent->scope = scope;
   ent->type  = type;
   ent->next  = ht_exists_sv(ht, name) ? ht_get_sv(ht, name) : NULL;

   ht_set_sv(ht, name, ent);
}

type_idx sym_table_get_cur(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf) {
   ht_t(wchar_t *, sym_entry_t *) ht = typedf ? tbl->types : (void *)tbl->decls;
   if (!ht_exists_sv(ht, name)) return -1;

   sym_entry_t *entry = ht_get_sv(ht, name);

   while (entry != NULL && entry->scope != scope) entry = entry->next;

   return entry != NULL ? entry->type : -1;
}

type_idx sym_table_get(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf) {
   ht_t(wchar_t *, sym_entry_t *) ht = typedf ? tbl->types : (void *)tbl->decls;
   if (!ht_exists_sv(ht, name)) return -1;

   sym_entry_t *fentry = ht_get_sv(ht, name); 

   sym_entry_t *entry = fentry;
   sym_entry_t *csco  = NULL;
   do if (entry->scope == scope) 
      csco = entry;
   while ((entry = entry->next) != NULL);

   if (csco == NULL) {
      if (!ht_exists(tbl->sinf, scope)) eprint("Invalid scope passed in!");

      struct sinf_t *pscope = ht_get(tbl->sinf, scope)->parent;
      
      return pscope == NULL ? -1 : sym_table_get(tbl, name, pscope->id, typedf);
   } else return csco->type;
}

// TODO: this should probably return the one with the closest scope
type_idx sym_table_get_cur_both(sym_table_t *tbl, wchar_t *name, size_t scope) {
   type_idx ret = sym_table_get_cur(tbl, name, scope, false);
   if (ret < 0) ret = sym_table_get_cur(tbl, name, scope, true);

   return ret;
}

type_idx sym_table_get_both(sym_table_t *tbl, wchar_t *name, size_t scope) {
   type_idx ret = sym_table_get(tbl, name, scope, false);
   if (ret < 0) ret = sym_table_get(tbl, name, scope, true);

   return ret;
}

void _sym_table_free(void *ent) {
   sym_entry_t *entry = ent;

   free(entry->name);

   if (entry->next != NULL) 
      _sym_table_free(entry->next);
}

void sym_table_free(sym_table_t *tbl) {
   ht_free_func(tbl->decls, _sym_table_free);
   ht_free_func(tbl->types, _sym_table_free);

   free(tbl);
}
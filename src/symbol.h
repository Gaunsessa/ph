#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <wchar.h>

#include <print.h>
#include <ht.h>
#include <dynarr.h>

#include <types.h>

// Todo setup more attributes then just type

typedef struct sym_entry_t {
   wchar_t *name;
   size_t scope;

   type_idx type;

   struct sym_entry_t *next;
} sym_entry_t;

typedef struct sym_table_t {
   ht_t(wchar_t *, sym_entry_t *) decls;
   ht_t(wchar_t *, sym_entry_t *) types;

   struct sinf_t {
      size_t id;
      struct sinf_t *parent; 
      dynarr_t(struct sinf_t *) children;
   } sinf_arr;
   ht_t(int, struct sinf_t *) sinf;
} sym_table_t;

sym_table_t *sym_table_new();

void sym_table_push_scope(sym_table_t *tbl, size_t scope, size_t parent);

void sym_table_set(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf, type_idx type);

type_idx sym_table_get_cur(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf);
type_idx sym_table_get(sym_table_t *tbl, wchar_t *name, size_t scope, bool typedf);

type_idx sym_table_get_cur_both(sym_table_t *tbl, wchar_t *name, size_t scope);
type_idx sym_table_get_both(sym_table_t *tbl, wchar_t *name, size_t scope);

void sym_table_free(sym_table_t *tbl);

#endif
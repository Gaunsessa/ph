#ifndef INFER_H
#define INFER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <ht.h>
#include <dynarr.h>
#include <print.h>

#include "util.h"
#include "node.h"
#include "symbol.h"
#include "types.h"

type_idx infer_expression(sym_table_t *tbl, sym_module_t *mod, size_t scope, node_t *expr);

#endif
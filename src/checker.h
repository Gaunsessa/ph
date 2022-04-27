#ifndef CHECKER_H
#define CHECKER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <dynarr.h>
#include <print.h>

#include "util.h"
#include "lexer.h"
#include "node.h"
#include "prims.h"
#include "error.h"

typedef struct checker_t {
   ht_t(char *, type_t *) file_decls;
   ht_t(char *, type_t *) scope_decls;

   size_t errors;
} checker_t;

bool checker_check(node_t *AST);

void checker_file(checker_t *ckr, node_t *file);

type_t *checker_infer_stmt_type(checker_t *ckr, node_t *stmt);

#endif
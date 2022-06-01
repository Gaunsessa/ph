#ifndef TYPERES_H
#define TYPERES_H

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
#include "lexer.h"
#include "node.h"

void typeres_resolve(node_t *AST);

bool typeres_special(node_t *node);
void typeres_start(node_t *node);
void typeres_end(node_t *node);

void typeres_data_type(node_t *type);

#endif
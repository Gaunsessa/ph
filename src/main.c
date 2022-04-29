#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "cgen.h"
#include "dynarr.h"
#include "ht.h"

#include "node.h"
#include "parser.h"
#include "lexer.h"
#include "checker.h"
#include "types.h"

#include "prims.h"
#include "print.h"
#include "util.h"

// Todo: 1) Checker & Types
//          Resolve all types
//          Errors
//       2) Unary Expressions
//       3) Structs
//       4) Modules
//       5) Strings
//       6) Arrays
//       7) Labeled breaks and continues
//       8) Curried Functions

void comple_file(const char *path, bool compl, bool pretty) {
   FILE *f = fopen(path, "r");

   fseek(f, 0, SEEK_END);
   size_t len = ftell(f);
   fseek(f, 0, SEEK_SET);

   char buf[len + 1];
   buf[len] = '\0';

   fread(buf, 1, len, f);
   fclose(f);

   node_t *AST = parser_parse(buf);

   checker_check(AST);

   // printf("%s\n", cgen_generate(AST));

   if (compl) {
      char *code = cgen_generate(AST);

      if (pretty) {
         // TODO: pretty print kills quotes
         char sysbuf[strlen("echo \"\"") + strlen(code) + strlen(" | clang-format") + 1];

         strcpy(sysbuf, "echo \'");
         strcpy(sysbuf + strlen(sysbuf), code);
         strcpy(sysbuf + strlen(sysbuf), "\' | clang-format");

         system(sysbuf);
      } else {
         printf("%s\n", code);
      }
   } else {
      print_node(AST);
      print("");
   }

   node_free(AST);
   type_free_all();
}

int main(int argc, char **argv) {
   type_module_init();
   lexer_module_init();

   comple_file("tests/test.ph", true, false);
}

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

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

// TODO: Sometimes im not freeing parser_identifier maybe others aswell

// Todo: 1) Checker & Types
//          Type Handler
//          Resolve all types
//          Errors
//       3) Structs
//       4) Modules
//       5) Strings
//       6) Arrays
//       7) Labeled breaks and continues
//       8) Curried Functions

void comple_file(const char *path, bool compl, bool pretty) {
   // FILE *f = fopen(path, "r");

   // fseek(f, 0, SEEK_END);
   // size_t len = ftell(f);
   // fseek(f, 0, SEEK_SET);

   // char cbuf[len + 4];
   // wchar_t *buf = (wchar_t *)cbuf;

   // for (int i = 0; true; i++) {
   //    buf[i] = fgetwc(f);

   //    if (buf[i] == WEOF) {
   //       buf[i] = 0;
   //       break;
   //    }
   // }

   // for (int i = 0; i < wcslen(buf); i++) {
   //    printf("%lc\n", buf[i]);
   // }

   // fclose(f);

   // lexer_t *lex = lexer_new(fopen(path, "r"));

   // while (lexer_has_more_tokens(lex)) {
   //    print_token(lexer_get_next_token(lex));
   //    // sleep(1);
   // }

   // return;

   node_t *AST = parser_parse(fopen(path, "r"));

   checker_check(AST);

   // printf("%s\n", cgen_generate(AST));

   if (compl) {
      wchar_t *code = cgen_generate(AST);

      if (pretty) {
         // TODO: pretty print kills quotes
         // wchar_t sysbuf[strlen("echo \"\"") + wcslen(code) + strlen(" | clang-format") + 1];

         // strcpy(sysbuf, "echo \'");
         // strcpy(sysbuf + wcslen(sysbuf), code);
         // strcpy(sysbuf + strlen(sysbuf), "\' | clang-format");

         // system(sysbuf);
      } else {
         printf("%ls\n", code);
      }
   } else {
      print_node(AST);
      print("");
   }

   node_free(AST);
   type_free_all();
}

int main(int argc, char **argv) {
   setlocale(LC_ALL, "");

   // printf("%ls\n", L"😳");

   type_module_init();
   lexer_module_init();

   comple_file("tests/test.ph", true, false);

   // print(ceil(log10(99)));u
   // print(9 / pow(10, ceil(log10(9))));
}

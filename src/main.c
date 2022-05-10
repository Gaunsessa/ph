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

// TODO: make variables and typedefs diffrent scopes!!!!!!!!!!!
// Todo: 1) Checker & Types
//          Type Handler
//          Errors
//          Struct funcs (impl and {...})
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

   // error_print_line(fopen(path, "r"), 1);

   // return;

   node_t *AST = parser_parse(fopen(path, "r"));

   // printf("%s\n", cgen_generate(AST));

   if (compl) {
      checker_check(AST);

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

typedef struct test_t {
   int a;
   int b;
   int c;
   int d;
   int e;
} test_t;

int main(int argc, char **argv) {
   setlocale(LC_ALL, "");

   // printf("%ls\n", L"😳");

   type_module_init();
   lexer_module_init();

   test_t t = (test_t) { 1, .d=2, 3, .b = 4, 5 };

   // print(t.a);
   // print(t.b);
   // print(t.c);
   // print(t.d);
   // print(t.e);

   comple_file("tests/test.ph", true, false);

   // float num = 1;

   // printf("%f\n", ceil(log10(num)));
   // printf("%f\n", floor(num / pow(10, ceil(log10(num)) - 1)));
   // printf("%f\n", num - pow(10, ceil(log10(num)) - 1) * floor(num / pow(10, ceil(log10(num)) - 1)));

   // print(ceil(log10(99)));u
   // print(9 / pow(10, ceil(log10(9))));
}
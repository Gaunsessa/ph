#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <setjmp.h>

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
//       type_t has memory leak

// TODO: 0) Desugar step
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

int compile_and_run(wchar_t *code) {
   int ccpipes[2][2];

   pipe(ccpipes[0]);
   pipe(ccpipes[1]);

   char tmp_path[] = "./XXXXXX";
   mkstemp(tmp_path);

   int proc = fork();

   if (proc < 0) {
      printf("Error: Failed to fork process!");
      exit(-1);
   }

   if (!proc) {
      close(ccpipes[0][1]);
      close(ccpipes[1][0]);
      
      dup2(ccpipes[0][0], STDIN_FILENO);
      dup2(ccpipes[1][1], STDOUT_FILENO);

      char buf[sizeof("gcc -o ./XXXXXX -w -xc -")];
      sprintf(buf, "gcc -o %s -w -xc -", tmp_path);

      execl("/bin/sh", "/bin/sh", "-c", buf, NULL);

      exit(0);
   } else {
      close(ccpipes[0][0]);
      close(ccpipes[1][1]);

      FILE *inp = fdopen(ccpipes[0][1], "w");
      FILE *out = fdopen(ccpipes[1][0], "r");

      fprintf(inp, "%ls", code);
      fclose(inp);

      FILE *tmp = fopen(tmp_path, "w");
      if (tmp == NULL) {
         printf("Error: Failed to create tmp file!");
         exit(-1);
      }

      char c;
      while ((c = fgetc(out)) != EOF)
         fputc(c, tmp);

      fclose(tmp);

      system(tmp_path);

      unlink(tmp_path);
   }

   return 0;
}

int main(int argc, char **argv) {
   setlocale(LC_ALL, "");

   type_module_init();
   lexer_module_init();

   if (argc < 3) { 
      printf("Usage: %s (build|run|dump) FILE.ph\n", argv[0]);
      exit(-1);
   }

   node_t *AST = parser_parse(fopen(argv[2], "r"));
   if (!checker_check(AST)) exit(-1);
   
   if (!strcmp(argv[1], "build")) {
      printf("%ls\n", cgen_generate(AST));
   } else if (!strcmp(argv[1], "run")) {
      compile_and_run(cgen_generate(AST));
   } else if (!strcmp(argv[1], "dump")) {
      print_node(AST);
   } else {
      printf("Invalid command: %s!\n", argv[1]);
      exit(-1);
   }

   node_free(AST);
   type_free_all();
}
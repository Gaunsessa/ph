#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "dynarr.h"
#include "ht.h"

#include "node.h"
#include "parser.h"
#include "lexer.h"
#include "types.h"
#include "symbol.h"
#include "cgen.h"

#include "prims.h"
#include "print.h"
#include "util.h"

#include "pass/init.h"
#include "pass/checker.h"
#include "pass/desugar.h"

// TODO: Sometimes im not freeing parser_identifier maybe others aswell
//       type_t has memory leak
//       node_free dosent free the malloc of the node itself

// TODO: 0) Desugar step
//          I think I should desugar after the check stage and desugar all modules down to a NODE_FILE with renamed names
//          and maybe even convert it into a new ast node instead of file that has a list of functions and variables and ...
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

dynarr_t(FILE *) load_files(int amt, char **paths, dynarr_t(FILE *) arr) {
   dynarr_t(FILE *) files = arr != NULL ? arr : dy_init(FILE *);

   struct stat sb;
   struct dirent *dir;

   for (int i = 0; i < amt; i++) {
      if (stat(paths[i], &sb) != 0) {
         printf("Error: %s: Unkown file!\n", paths[i]);
         exit(-1);
      }

      if (S_ISREG(sb.st_mode)) {
         dy_push(files, fopen(paths[i], "r"));
      } else if (S_ISDIR(sb.st_mode)) {
         if (!strcmp(paths[i], ".") || !strcmp(paths[i], "..")) continue;

         DIR *d = opendir(paths[i]);
         if (d == NULL) continue;

         chdir(paths[i]);

         dynarr_t(char *) dpaths = dy_init(char *);

         while ((dir = readdir(d)) != NULL) {
            char *p = malloc(dir->d_namlen + 1);
            strcpy(p, dir->d_name);

            dy_push(dpaths, p);
         }

         load_files(dy_len(dpaths), dyi(dpaths), files);

         for (int i = 0; i < dy_len(dpaths); i++)
            free(dyi(dpaths)[i]);

         dy_free(dpaths);

         chdir("..");

         closedir(d);
      } else {
         printf("Error: %s: Unkown file type!\n", paths[i]);
         exit(-1);
      }
   }

   return files;
}

int main(int argc, char **argv) {
   setlocale(LC_ALL, "");

   type_module_init();
   lexer_module_init();

   if (argc < 3) { 
      printf("Usage: %s (build|run|dump) (FILE(s).ph|DIR)\n", argv[0]);
      exit(-1);
   }

   type_handler_t *type_handler = type_handler_new();
   sym_table_t *symtbl = sym_table_new();

   // sym_table_push_scope(symtbl, 1, 0);
   // sym_table_push_scope(symtbl, 2, 0);
   // sym_table_push_scope(symtbl, 3, 1);

   // sym_table_set(symtbl, L"joe", 0, false, 69);
   // sym_table_set(symtbl, L"joe", 1, false, 420);

   // print(sym_table_get(symtbl, L"joe", 3, false));


   // return 0;

   dynarr_t(FILE *) files = load_files(argc - 2, argv + 2, NULL);

   node_t *AST = parser_parse(dy_len(files), dyi(files));

   dy_free(files);

   init_pass(AST, symtbl);

   print(sym_table_get(symtbl, L"fewfe", 0, false));

   return 0;

   // if (!checker_check(AST, type_handler)) exit(-1);

   // node_t *NAST = desugar_desugar(AST);
   node_t *NAST = AST;
   
   if (!strcmp(argv[1], "build")) {
      // printf("%ls\n", cgen_generate(NAST));
   } else if (!strcmp(argv[1], "run")) {
      // compile_and_run(cgen_generate(NAST));
   } else if (!strcmp(argv[1], "dump")) {
      print_node(AST);
   } else {
      printf("Invalid command: %s!\n", argv[1]);
      exit(-1);
   }

   node_free(AST);
   type_handler_free(type_handler);
}
#ifndef ERROR_H
#define ERROR_H

#include "lexer.h"

static inline void error_print_line(FILE *f, size_t line) {
   fseek(f, 0, SEEK_SET);

   size_t l = 0;
   while (true) {
      if (l == line - 1) {
         size_t len;
         wchar_t *t = fgetwln(f, &len);

         for (int i = 0; i < len; i++)
            printf("%lc", t[i]);

         if (t[len - 1] != '\n') printf("\n");

         return;
      }

      wint_t c = fgetwc(f);
      if (c == WEOF) eprint("Line not found: %lu!", line);

      if (c == '\n') l++;
   }
}

__attribute__((noreturn)) static inline void error_token(lexer_t *lexer, token_t token, const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);

   printf("test.ph:%lu:%lu: ", token.line, token.pos);
   vprintf(fmt, args);
   printf("\n");
   
   error_print_line(fdopen(fileno(lexer->file), "r"), token.line);
   for (int i = 0; i < token.pos - 1; i++) printf(" ");
   printf("^\n");

   print_token_type(token.type);

   exit(-1);
}

#endif
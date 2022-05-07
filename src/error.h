#ifndef ERROR_H
#define ERROR_H

#include "lexer.h"

__attribute__((noreturn)) static inline void error_token(lexer_t *lexer, token_t token, const char *msg) {
   printf("test.ph:%lu:0:", token.line);
   print_token(token);
   printf(": %s\n", msg);

   // char *pos = lexer->buf + token.pos;

   // while (pos != lexer->buf && *pos != '\n') pos--;

   // pos++;

   // while (pos - lexer->buf < lexer->buf_len && *pos != '\n') {
   //    printf("%c", *pos);
   //    pos++;
   // }

   // printf("\n");

   exit(-1);
}

#endif
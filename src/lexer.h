#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include <print.h>
#include <ht.h>

#include "util.h"

#define TOKEN_TYPES                     \
      TOKEN(NONE, "")                   \
      TOKEN(END, "")                    \
      TOKEN(STRING, "")                 \
      TOKEN(NUMBER, "")                 \
      TOKEN(IDENTIFIER, "")             \
                                        \
      TOKEN(NEWLINE, "\n")              \
      TOKEN(SEMI_COLON, ";")            \
      TOKEN(COMMA, ",")                 \
      TOKEN(BACK_SLASH, "\\")           \
      TOKEN(UNDERSCORE, "_")            \
                                        \
      TOKEN(PLUS, "+")                  \
      TOKEN(MINUS, "-")                 \
      TOKEN(ASTERISK, "*")              \
      TOKEN(FORWARD_SLASH, "/")         \
      TOKEN(MOD, "%")                   \
      TOKEN(PLUS_PLUS, "++")            \
      TOKEN(MINUS_MINUS, "--")          \
      TOKEN(PLUS_EQUALS, "+=")          \
      TOKEN(MINUS_EQUALS, "-=")         \
      TOKEN(ASTERISK_EQUALS, "*=")      \
      TOKEN(FORWARD_SLASH_EQUALS, "/=") \
      TOKEN(MOD_EQUALS, "%=")           \
                                        \
      TOKEN(BANG, "!")                  \
      TOKEN(GREATER_THAN, ">")          \
      TOKEN(LESS_THAN, "<")             \
      TOKEN(GREATER_THAN_EQUALS, ">=")  \
      TOKEN(LESS_THAN_EQUALS, "<=")     \
      TOKEN(EQUALS_EQUALS, "==")        \
      TOKEN(NOT_EQUALS, "!=")           \
      TOKEN(OR_OR, "||")                \
      TOKEN(AND_AND, "&&")              \
                                        \
      TOKEN(CARET, "^")                 \
      TOKEN(AND, "&")                   \
      TOKEN(OR, "|")                    \
      TOKEN(LEFT_SHIFT, "<<")           \
      TOKEN(RIGHT_SHIFT, ">>")          \
      TOKEN(CARET_EQUALS, "^=")         \
      TOKEN(AND_EQUALS, "&=")           \
      TOKEN(OR_EQUALS, "|=")            \
      TOKEN(LEFT_SHIFT_EQUALS, "<<=")   \
      TOKEN(RIGHT_SHIFT_EQUALS, ">>=")  \
                                        \
      TOKEN(LEFT_PARENTHESES, "(")      \
      TOKEN(RIGHT_PARENTHESES, ")")     \
      TOKEN(LEFT_BRACE, "{")            \
      TOKEN(RIGHT_BRACE, "}")           \
      TOKEN(LEFT_BRACKET, "[")          \
      TOKEN(RIGHT_BRACKET, "]")         \
      TOKEN(COLON, ":")                 \
      TOKEN(EQUALS, "=")                \
      TOKEN(ARROW, "->")                \
                                        \
      TOKEN(IF, "if")                   \
      TOKEN(ELSE, "else")               \
      TOKEN(FOR, "for")                 \
      TOKEN(BREAK, "break")             \
      TOKEN(CONTINUE, "continue")       \
      TOKEN(RETURN, "return")           \
   

const static size_t MAX_KEYWORD_LEN = 32;

typedef struct lexer_t {
   size_t buf_len;
   char *buf;

   size_t line;

   size_t cursor;
} lexer_t;

typedef enum TOKEN_TYPE {
   _TOKEN_ = -2,
#define TOKEN(type, ident) TOKEN_##type,
   TOKEN_TYPES
#undef TOKEN
} TOKEN_TYPE;

typedef struct token_t {
   TOKEN_TYPE type;

   size_t line;
   size_t pos;

   union {
      // String | Identifier
      char *str;

      // Number
      struct {
         size_t num;
      };
   };
} token_t;

ht_t(char *, TOKEN_TYPE) TOKEN_IDENTS;
ht_t(TOKEN_TYPE, char *) TOKEN_STRS;

void lexer_module_init();

lexer_t *lexer_new(char *str, size_t str_len);

token_t lexer_get_next_token(lexer_t *lexer);
bool lexer_has_more_tokens(lexer_t *lexer);

void lexer_free(lexer_t *lexer);

void print_token(token_t token);
void print_token_type(TOKEN_TYPE t);

#endif

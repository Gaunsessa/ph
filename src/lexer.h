#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <wchar.h>

#include <print.h>
#include <ht.h>
#include <dynarr.h>
#include <utf8proc/utf8proc.h>

#include "util.h"

#define TOKEN_TYPES                      \
      TOKEN(NONE, L"")                   \
      TOKEN(END, L"")                    \
      TOKEN(STRING, L"")                 \
      TOKEN(NUMBER, L"")                 \
      TOKEN(FLOAT, L"")                  \
      TOKEN(HEX, L"")                    \
      TOKEN(IDENTIFIER, L"")             \
                                         \
      TOKEN(TRUE, L"true")               \
      TOKEN(FALSE, L"false")             \
                                         \
      TOKEN(NEWLINE, L"\n")              \
      TOKEN(SEMI_COLON, L";")            \
      TOKEN(COMMA, L",")                 \
      TOKEN(BACK_SLASH, L"\\")           \
      TOKEN(UNDERSCORE, L"_")            \
                                         \
      TOKEN(PLUS, L"+")                  \
      TOKEN(MINUS, L"-")                 \
      TOKEN(ASTERISK, L"*")              \
      TOKEN(FORWARD_SLASH, L"/")         \
      TOKEN(MOD, L"%")                   \
      TOKEN(PLUS_PLUS, L"++")            \
      TOKEN(MINUS_MINUS, L"--")          \
      TOKEN(PLUS_EQUALS, L"+=")          \
      TOKEN(MINUS_EQUALS, L"-=")         \
      TOKEN(ASTERISK_EQUALS, L"*=")      \
      TOKEN(FORWARD_SLASH_EQUALS, L"/=") \
      TOKEN(MOD_EQUALS, L"%=")           \
                                         \
      TOKEN(BANG, L"!")                  \
      TOKEN(GREATER_THAN, L">")          \
      TOKEN(LESS_THAN, L"<")             \
      TOKEN(GREATER_THAN_EQUALS, L">=")  \
      TOKEN(LESS_THAN_EQUALS, L"<=")     \
      TOKEN(EQUALS_EQUALS, L"==")        \
      TOKEN(NOT_EQUALS, L"!=")           \
      TOKEN(OR_OR, L"||")                \
      TOKEN(AND_AND, L"&&")              \
                                         \
      TOKEN(CARET, L"^")                 \
      TOKEN(AND, L"&")                   \
      TOKEN(OR, L"|")                    \
      TOKEN(TILDE, L"~")                 \
      TOKEN(LEFT_SHIFT, L"<<")           \
      TOKEN(RIGHT_SHIFT, L">>")          \
      TOKEN(CARET_EQUALS, L"^=")         \
      TOKEN(AND_EQUALS, L"&=")           \
      TOKEN(OR_EQUALS, L"|=")            \
      TOKEN(LEFT_SHIFT_EQUALS, L"<<=")   \
      TOKEN(RIGHT_SHIFT_EQUALS, L">>=")  \
                                         \
      TOKEN(LEFT_PARENTHESES, L"(")      \
      TOKEN(RIGHT_PARENTHESES, L")")     \
      TOKEN(LEFT_BRACE, L"{")            \
      TOKEN(RIGHT_BRACE, L"}")           \
      TOKEN(LEFT_BRACKET, L"[")          \
      TOKEN(RIGHT_BRACKET, L"]")         \
      TOKEN(COLON, L":")                 \
      TOKEN(EQUALS, L"=")                \
      TOKEN(ARROW, L"->")                \
      TOKEN(DOT, L".")                   \
      TOKEN(APOSTROOHE, L"'")            \
      TOKEN(UNINIT, L"---")              \
                                         \
      TOKEN(IF, L"if")                   \
      TOKEN(ELSE, L"else")               \
      TOKEN(FOR, L"for")                 \
      TOKEN(DOR, L"dor")                 \
      TOKEN(BREAK, L"break")             \
      TOKEN(CONTINUE, L"continue")       \
      TOKEN(RETURN, L"return")           \
      TOKEN(ALIAS, L"alias")             \
      TOKEN(STRUCT, L"struct")           \
      TOKEN(IMPL, L"impl")               \
      TOKEN(DEFER, L"defer")             \
      TOKEN(MODULE, L"module")           \
      TOKEN(IMPORT, L"import")           \

const static size_t MAX_KEYWORD_LEN = 32;

typedef struct lexer_t {
   FILE *file;
   wchar_t buf[MAX_KEYWORD_LEN];
   bool end;

   size_t line;
   size_t pos;

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
      wchar_t *str;

      // Number
      struct {
         size_t num;
      };

      // Float
      struct {
         size_t integer;
         size_t fraction;
      };
   };
} token_t;

ht_t(wchar_t *, TOKEN_TYPE) TOKEN_IDENTS;
ht_t(TOKEN_TYPE, wchar_t *) TOKEN_STRS;

void lexer_module_init();

lexer_t *lexer_new(FILE *file);

token_t lexer_get_next_token(lexer_t *lexer);
bool lexer_has_more_tokens(lexer_t *lexer);

size_t lexer_num(lexer_t *lexer);
size_t lexer_hex(lexer_t *lexer);
size_t lexer_bin(lexer_t *lexer);

void lexer_free(lexer_t *lexer);

void print_token(token_t token);
void print_token_type(TOKEN_TYPE t);
const char *token_type_str(TOKEN_TYPE t);

#endif

#include "lexer.h"

#define TOKEN(type, ident) {ident, TOKEN_##type},
__typeof__(TOKEN_IDENTS) TOKEN_IDENTS = ht_c_sv(wchar_t, TOKEN_TYPE, 
   TOKEN_TYPES
);
#undef TOKEN

#define TOKEN(type, ident) {TOKEN_##type, ident},
__typeof__(TOKEN_STRS) TOKEN_STRS = ht_c_vs(TOKEN_TYPE, wchar_t, 
   TOKEN_TYPES
);
#undef TOKEN

#define gettoken(lexer, t) lexer->buf[(lexer->cursor + t) % MAX_KEYWORD_LEN]
#define curtoken(lexer) gettoken(lexer, 0)
#define nextoken(lexer) gettoken(lexer, 1)

void lexer_module_init() {
   TOKEN_IDENTS = ht_init_sv(wchar_t, TOKEN_TYPE, TOKEN_IDENTS);
   TOKEN_STRS = ht_init_vs(TOKEN_TYPE, wchar_t, TOKEN_STRS);
}

wchar_t lexer_read_wchar(lexer_t *lexer) {
   if (lexer->end) return 0;

   wint_t c = fgetwc(lexer->file);

   if (c == WEOF) {
      lexer->end = true;
      return 0;
   } else return c;
}

void lexer_inc_cursor(lexer_t *lexer, int amt) {
   for (int i = 0; i < amt; i++)
      gettoken(lexer, i) = lexer_read_wchar(lexer);

   lexer->cursor = lexer->cursor + amt % MAX_KEYWORD_LEN;
}

lexer_t *lexer_new(FILE *f) {
   lexer_t *lexer = calloc(1, sizeof(lexer_t));

   lexer->file = f;
   lexer->line = 1;

   for (int i = 0; i < MAX_KEYWORD_LEN; i++)
      lexer->buf[i] = lexer_read_wchar(lexer);

   return lexer;
}

bool lexer_has_more_tokens(lexer_t *lexer) {
   return curtoken(lexer) != 0;
}

token_t lexer_get_next_token(lexer_t *lexer) {
   if (!lexer_has_more_tokens(lexer)) return (token_t) { TOKEN_END };

   // Newline
   if (curtoken(lexer) == '\n') {
      lexer_inc_cursor(lexer, 1);
      lexer->line++;

      return (token_t) { TOKEN_NEWLINE, lexer->line, lexer->cursor };
   }

   // Whitespace
   while (curtoken(lexer) != '\n' && isspace(curtoken(lexer))) lexer_inc_cursor(lexer, 1);

   // Comments
   if (curtoken(lexer) == '/' && nextoken(lexer) == '/') {
      while (curtoken(lexer) != '\n') 
         if (!lexer_has_more_tokens(lexer)) return (token_t) { TOKEN_END };
         else lexer_inc_cursor(lexer, 1);

      lexer_inc_cursor(lexer, 1);

      return (token_t) { TOKEN_NEWLINE, lexer->line, lexer->cursor };
   }

   // Numbers
   if (curtoken(lexer) == '0' && nextoken(lexer) == 'x') {
      lexer_inc_cursor(lexer, 2);

      return (token_t) { TOKEN_NUMBER, lexer->line, lexer->cursor, { .num = lexer_hex(lexer) } };
   }

   if (curtoken(lexer) == '0' && nextoken(lexer) == 'b') {
      lexer_inc_cursor(lexer, 2);

      return (token_t) { TOKEN_NUMBER, lexer->line, lexer->cursor, { .num = lexer_bin(lexer) } };
   }

   if (isdigit(curtoken(lexer))) {
      size_t num = lexer_num(lexer);

      if (curtoken(lexer) == '.') {
         lexer_inc_cursor(lexer, 1);

         return (token_t) { TOKEN_FLOAT, lexer->line, lexer->cursor, { .integer = num, .fraction = lexer_num(lexer) } };
      } else return (token_t) { TOKEN_NUMBER, lexer->line, lexer->cursor, { .num = num } };
   }

   // Strings
   // TODO: Escape chars
   if (curtoken(lexer) == '"') {
      lexer_inc_cursor(lexer, 1);

      dynarr_t(wchar_t) str = dy_init(wchar_t);

      while (curtoken(lexer) != '"') {
         dy_push(str, curtoken(lexer));
         lexer_inc_cursor(lexer, 1);
      }

      dy_push(str, 0);
      lexer_inc_cursor(lexer, 1);

      token_t token = (token_t) { 
         TOKEN_STRING, 
         lexer->line, 
         lexer->cursor, 
         { .str = dup_mem(dyi(str), dy_len(str) * sizeof(wchar_t)) } 
      };

      dy_free(str);

      return token;
   }

   if (curtoken(lexer) == '_' && M_COMPARE(utf8proc_category(nextoken(lexer)), 
                                           UTF8PROC_CATEGORY_LU, UTF8PROC_CATEGORY_LL, UTF8PROC_CATEGORY_LT,
                                           UTF8PROC_CATEGORY_LM, UTF8PROC_CATEGORY_LO, UTF8PROC_CATEGORY_NL,
                                           UTF8PROC_CATEGORY_SO)) goto IDENT;

   // Keywords
   TOKEN_TYPE cur = TOKEN_NONE;
   wchar_t key[MAX_KEYWORD_LEN];
   size_t key_index = 0;

   for (int i = 1; i < MAX_KEYWORD_LEN; i++) {
      key[i] = 0;
      key[i - 1] = gettoken(lexer, i - 1);

      if (ht_exists_sv(TOKEN_IDENTS, key)) {
         cur = ht_get_sv(TOKEN_IDENTS, key);
         key_index = i;

         continue;
      } else if (cur != TOKEN_NONE) break;
   }

   if (cur != TOKEN_NONE) {
      lexer_inc_cursor(lexer, key_index);

      return (token_t) { cur, lexer->line, lexer->cursor };
   }

IDENT:
   // Identifier
   if (M_COMPARE(utf8proc_category(curtoken(lexer)), 
       UTF8PROC_CATEGORY_LU, UTF8PROC_CATEGORY_LL, UTF8PROC_CATEGORY_LT,
       UTF8PROC_CATEGORY_LM, UTF8PROC_CATEGORY_LO, UTF8PROC_CATEGORY_NL,
       UTF8PROC_CATEGORY_SO) || curtoken(lexer) == '_') {
      dynarr_t(wchar_t) str = dy_init(wchar_t);

      dy_push(str, curtoken(lexer));
      lexer_inc_cursor(lexer, 1);

      while (M_COMPARE(utf8proc_category(curtoken(lexer)), 
             UTF8PROC_CATEGORY_LU, UTF8PROC_CATEGORY_LL, UTF8PROC_CATEGORY_LT,
             UTF8PROC_CATEGORY_LM, UTF8PROC_CATEGORY_LO, UTF8PROC_CATEGORY_NL,
             UTF8PROC_CATEGORY_MN, UTF8PROC_CATEGORY_MC, UTF8PROC_CATEGORY_ND,
             UTF8PROC_CATEGORY_SO) || curtoken(lexer) == '_') {
         dy_push(str, curtoken(lexer));
         lexer_inc_cursor(lexer, 1);
      }

      dy_push(str, 0);

      token_t token = (token_t) { 
         TOKEN_IDENTIFIER, 
         lexer->line, 
         lexer->cursor, 
         { .str = dup_mem(dyi(str), dy_len(str) * sizeof(wchar_t)) } 
      };

      return token;
   }

   return (token_t) { TOKEN_NONE };
}

void lexer_free(lexer_t *lexer) {
   free(lexer->buf);
   free(lexer);
}

size_t lexer_num(lexer_t *lexer) {
   size_t num = 0;

   do {
      num *= 10;
      num += curtoken(lexer) - '0';

      lexer_inc_cursor(lexer, 1);
   } while (isdigit(curtoken(lexer)));

   return num;
}

size_t lexer_hex(lexer_t *lexer) {
   size_t num = 0;

   for (char inp = curtoken(lexer); isdigit(inp) || (inp >= 'a' && inp <= 'f') || (inp >= 'A' && inp <= 'F');) {
      num *= 16;

      if (isdigit(inp)) num += inp - '0';
      else num += inp - ((inp <= 'F') ? 'A' : 'a') + 10;

      lexer_inc_cursor(lexer, 1);
      inp = curtoken(lexer);
   }

   return num;
}

size_t lexer_bin(lexer_t *lexer) {
   size_t num = 0;

   do {
      num *= 2;
      num += curtoken(lexer) - '0';

      lexer_inc_cursor(lexer, 1);
   } while (curtoken(lexer) == '1' || curtoken(lexer) == '0');

   return num;
}

void print_token(token_t token) {
   switch (token.type) {
      case TOKEN_NONE:
         printf("TOKEN: NONE\n");
         break;
      case TOKEN_END:
         printf("TOKEN: END\n");
         break;
      case TOKEN_NEWLINE:
         printf("TOKEN: NEWLINE\n");
         break;
      case TOKEN_STRING:
         printf("TOKEN: STRING | \"%ls\"\n", token.str);
         break;
      case TOKEN_NUMBER:
         printf("TOKEN: NUMBER | %lu\n", token.num);
         break;
      case TOKEN_FLOAT:
         printf("TOKEN: FLOAT | %lu.%lu\n", token.integer, token.fraction);
         break;
      case TOKEN_IDENTIFIER:
         printf("TOKEN: IDENTIFIER | %ls\n", token.str);
         break;
      default:
         printf("TOKEN: KEYWORD | ");
         printf("%ls | %d\n", ht_get_vs(TOKEN_STRS, token.type), token.type);

         break;
   }
}

void print_token_type(TOKEN_TYPE t) {
   switch (t) {
#define TOKEN(type, ident) case TOKEN_##type: printf("%s", M_STR(type)); break;
      TOKEN_TYPES
#undef TOKEN

      default: return;
   }
}

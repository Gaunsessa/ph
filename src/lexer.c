#include "lexer.h"

#define TOKEN(type, ident) {ident, TOKEN_##type},
__typeof__(TOKEN_IDENTS) TOKEN_IDENTS = ht_c_sv(TOKEN_TYPE, 
   TOKEN_TYPES
);
#undef TOKEN

#define TOKEN(type, ident) {TOKEN_##type, ident},
__typeof__(TOKEN_STRS) TOKEN_STRS = ht_c_vs(TOKEN_TYPE, 
   TOKEN_TYPES
);
#undef TOKEN

#define curtoken(lexer) lexer->buf[lexer->cursor]
#define nextoken(lexer) lexer->buf[lexer->cursor + 1]

void lexer_module_init() {
   TOKEN_IDENTS = ht_init_sv(TOKEN_TYPE, TOKEN_IDENTS);
   TOKEN_STRS = ht_init_vs(TOKEN_TYPE, TOKEN_STRS);
}

lexer_t *lexer_new(char *str, size_t str_len) {
   lexer_t *lexer = calloc(1, sizeof(lexer_t));

   lexer->buf = malloc(str_len + 1);
   lexer->buf_len = str_len;

   lexer->line = 1;

   memcpy(lexer->buf, str, str_len);

   return lexer;
}

bool lexer_has_more_tokens(lexer_t *lexer) {
   return lexer->cursor < lexer->buf_len;
}

token_t lexer_get_next_token(lexer_t *lexer) {
   if (!lexer_has_more_tokens(lexer)) return (token_t) { TOKEN_END };

   // Newline
   if (curtoken(lexer) == '\n') {
      lexer->cursor++;
      lexer->line++;

      return (token_t) { TOKEN_NEWLINE, lexer->line, lexer->cursor };
   }

   // Whitespace
   while (curtoken(lexer) != '\n' && isspace(curtoken(lexer))) lexer->cursor++;

   // Comments
   if (curtoken(lexer) == '/' && nextoken(lexer) == '/') {
      while (curtoken(lexer) != '\n') 
         if (!lexer_has_more_tokens(lexer)) return (token_t) { TOKEN_END };
         else lexer->cursor++;

      lexer->cursor++;

      return (token_t) { TOKEN_NEWLINE, lexer->line, lexer->cursor };
   }

   // Numbers
   if (isdigit(curtoken(lexer))) {
      size_t num = 0;
      size_t itr = 0;

      do {
         num *= 10;
         num += curtoken(lexer) - '0';

         itr++;
         lexer->cursor++;
      } while (isdigit(curtoken(lexer)));

      return (token_t) { TOKEN_NUMBER, lexer->line, lexer->cursor, { .num = num } };
   }

   // Strings
   // TODO: Escape chars
   if (curtoken(lexer) == '"') {
      int i = 1;
      while (lexer->buf[lexer->cursor + i] != '"') i++;

      token_t token = (token_t) { TOKEN_STRING, lexer->line, lexer->cursor, { .span.ptr = lexer->buf + lexer->cursor + 1, .span.len = i - 1 } };

      lexer->cursor += i + 1;

      return token;
   }

   TOKEN_TYPE cur = TOKEN_NONE;
   char key[MAX_KEYWORD_LEN];
   size_t key_index = 0;

   for (int i = 1; i < MAX_KEYWORD_LEN && lexer->cursor + i <= lexer->buf_len; i++) {
      key[i] = '\0';
      key[i - 1] = lexer->buf[lexer->cursor + i - 1];

      if (ht_exists_sv(TOKEN_IDENTS, key)) {
         cur = ht_get_sv(TOKEN_IDENTS, key);
         key_index = i;

         continue;
      }
   }

   if (cur != TOKEN_NONE) {
      lexer->cursor += key_index;

      return (token_t) { cur, lexer->line, lexer->cursor };
   }

   // Identifier
   if (isalnum(curtoken(lexer)) || (curtoken(lexer) == '_' && isalnum(nextoken(lexer)))) {
      int i = 0;
      while (isalnum(lexer->buf[lexer->cursor + i]) || lexer->buf[lexer->cursor + i] == '_') i++;

      token_t token = (token_t) { TOKEN_IDENTIFIER, lexer->line, lexer->cursor, { .span.ptr = lexer->buf + lexer->cursor, .span.len = i } };

      lexer->cursor += i;

      return token;
   }

   return (token_t) { TOKEN_NONE };
}

void lexer_free(lexer_t *lexer) {
   free(lexer->buf);
   free(lexer);
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
         printf("TOKEN: STRING | \"");

         print_span(&token.span);

         printf("\"\n");

         break;
      case TOKEN_NUMBER:
         printf("TOKEN: NUMBER | %lu\n", token.num);
         break;
      case TOKEN_IDENTIFIER:
         printf("TOKEN: IDENTIFIER | ");

         print_span(&token.span);

         printf("\n");

         break;
      default:
         printf("TOKEN: KEYWORD | ");
         printf("%s | %d\n", ht_get_vs(TOKEN_STRS, token.type), token.type);

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

#include "parser.h"

/*
   FILE
      | VariableDeclaration
      | Empty
      ;

   Statement*
      : Expression
      | VariableDeclaration
      | FunctionDeclaration
      | Block
      | If
      | Return
      ;

   Expression*
      : Literal
      | Identifier
      | BinaryExpression
      | UnaryExpression
      | CallExpression
      ;

   If
      : 'if' Expression Block (Else)
      | 'if' Expression '->' Statement (Else)
      ;

   Else*
      | 'else' Statement
      ;

   For
      : 'for' (Statement) ';' (Statement) ';' (Statement) ('->' Statement) | Block
      | 'for' Statement ('->' Statement) | Block
      | 'for' ('->' Statement) | Block
      ;

   Type*
      : BaseType
      | PtrType
      | ArrayType
      | FunctionType
      ;

   BaseType
      : STRING
      ;

   PtrType
      : '^' Type
      ;

   ArrayType
      : '[' NUMBER ']' Type
      ;

   FunctionType
      : '(' ')' ':' (Type)
      | '(' Identifier ':' Types(,) ')' ':' (Type)
      ;

   BinaryExpression
      : Expression '+'|'-'|'*'|'/' ('=') Expression
      ;

   UnaryExpression
      : Identifier '++'|'--'
      | '++'|'--' Identifier
      ;

   CallExpression
      : Identifier ('!') '(' Expression(,) ')'
      ; 

   VariableDeclaration
      : Identifiers(,) ':' Types(,)
      | Identifiers(,) ':' Types|'_'(,) '=' Expressions(,)
      | Identifiers(,) ':' '=' Expressions(,)
      ;

   FunctionDeclaration
      : FunctionType '=' Statement
      ;

   Return
      : 'return' Expression
      ;

   Literal*
      : NUMBER
      | STRING
      ;

   Identifier
      : NAME
      ;

   Block
      : '{' Statements '}'
      ;
*/

#define lookahead(n) p->lookaheads[n]
#define parser_semi(p) ({ if (p->req_semi) parser_eat(p, TOKEN_NEWLINE, TOKEN_SEMI_COLON); p->req_semi = true; })

/* ---- 
   MAIN 
   ---- */

node_t *parser_parse(char *str) {
   parser_t *inner = calloc(1, sizeof(parser_t));

   inner->lexer = lexer_new(str, strlen(str));
   inner->lookaheads[0] = lexer_get_next_token(inner->lexer);
   inner->lookaheads[1] = lexer_get_next_token(inner->lexer);
   inner->req_semi = true;

   node_t *ast = parser_file(inner);

   // TODO: Freeing of the parser
   // lexer_free(inner->lexer);
   // free(inner);

   return ast;
}

/* ---- 
   FILE 
   ---- */

node_t *parser_file(parser_t *p) {
   node_t *file = node_init(NODE_FILE, .FILE = { "main", dy_init(node_t *) });

   while (!(M_COMPARE(lookahead(0).type, TOKEN_END))) { 
      if (lookahead(0).type == TOKEN_NONE)
         break;

      node_t *node;

      switch (lookahead(0).type) {
         case TOKEN_UNDERSCORE:
         case TOKEN_IDENTIFIER:
            switch (lookahead(1).type) {
               case TOKEN_COLON: node = parser_variable_declaration(p); goto SWITCHEND;
               case TOKEN_COMMA: node = parser_multi_variable_declaration(p); goto SWITCHEND;
               default: goto DEFAULT;
            }
            goto SWITCHEND;
         case TOKEN_SEMI_COLON:
         case TOKEN_NEWLINE:
            node = node_init(NODE_EMPTY);
            goto SWITCHEND;
DEFAULT:
         default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
      }

SWITCHEND:
      if (node->type == NODE_MULTI) {
         for (int i = 0; i < dy_len(node->MULTI.nodes); i++)
            dy_push(file->FILE.stmts, dyi(node->MULTI.nodes)[i]);

         dy_free(node->MULTI.nodes);
         free(node);
      } else dy_push(file->FILE.stmts, node);

      if (M_COMPARE(lookahead(0).type, TOKEN_END, TOKEN_NONE)) break;

      parser_eat(p, TOKEN_NEWLINE);
   }

   return file;
}

/* --------- 
   STATEMENT 
   --------- */

node_t *parser_statement(parser_t *p) {
   switch (lookahead(0).type) {
      case TOKEN_LEFT_PARENTHESES:
         return parser_parenthesized_statement(p);
      case TOKEN_STRING:
      case TOKEN_NUMBER:
      // case TOKEN_PLUS_PLUS:
      // case TOKEN_MINUS_MINUS:
         return parser_expression(p, 0);
      case TOKEN_UNDERSCORE:
      case TOKEN_IDENTIFIER:
         switch (lookahead(1).type) {
            case TOKEN_COLON: return parser_variable_declaration(p);
            case TOKEN_COMMA: return parser_multi_variable_declaration(p);
            default:          return parser_expression(p, 0);
         }
      case TOKEN_LEFT_BRACE:
         return parser_block(p);
      case TOKEN_IF:
         return parser_if(p);
      case TOKEN_FOR:
         return parser_for(p);
      case TOKEN_BREAK:
         return parser_break(p);
      case TOKEN_CONTINUE:
         return parser_continue(p);
      case TOKEN_RETURN:
         return parser_return(p);
      case TOKEN_SEMI_COLON:
      case TOKEN_NEWLINE:
         return node_init(NODE_EMPTY);
      default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
   }
}

// This is bad...
node_t *parser_parenthesized_statement(parser_t *p) {
   int i = 1;

   while (p->lexer->buf[p->lexer->cursor + i] != '(') i--;
   while (p->lexer->buf[p->lexer->cursor + i] != ')') i++;

   switch (p->lexer->buf[p->lexer->cursor + i + 1]) {
      case ':': return parser_function_declaration(p);
      default: return parser_expression(p, 0);
   }
}

/* ---------- 
   EXPRESSION 
   ---------- */

#define _binary_expression(precedence, ...)                                 \
   ({                                                                       \
      case precedence:                                                      \
         expr = parser_expression(p, precedence + 1);                       \
         while (M_COMPARE(lookahead(0).type, __VA_ARGS__)) {                \
            expr = node_init(NODE_BINARY_EXPRESSION, .BINARY_EXPRESSION = { \
               parser_eat(p, __VA_ARGS__).type,                             \
               expr,                                                        \
               parser_expression(p, precedence + 1)                         \
            });                                                             \
         }                                                                  \
         break;                                                             \
   })                                                                       \

node_t *parser_expression(parser_t *p, int precedence) {
   // if (M_COMPARE(lookahead(0).type, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS) || 
   //    M_COMPARE(lookahead(1).type, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS))
   //    return parser_unary_expression(p);

   node_t *expr = NULL;

   switch (precedence) {
      _binary_expression(0, TOKEN_EQUALS,            TOKEN_PLUS_EQUALS,          TOKEN_MINUS_EQUALS, 
                            TOKEN_ASTERISK_EQUALS,   TOKEN_FORWARD_SLASH_EQUALS, TOKEN_MOD_EQUALS, 
                            TOKEN_LEFT_SHIFT_EQUALS, TOKEN_RIGHT_SHIFT_EQUALS,   TOKEN_AND_EQUALS, 
                            TOKEN_CARET_EQUALS,      TOKEN_OR_EQUALS);
      _binary_expression(1, TOKEN_OR_OR);
      _binary_expression(2, TOKEN_AND_AND);
      _binary_expression(3, TOKEN_OR);
      _binary_expression(4, TOKEN_CARET);
      _binary_expression(5, TOKEN_AND);
      _binary_expression(6, TOKEN_EQUALS_EQUALS, TOKEN_NOT_EQUALS);
      _binary_expression(7, TOKEN_LESS_THAN, TOKEN_GREATER_THAN, TOKEN_LESS_THAN_EQUALS, TOKEN_GREATER_THAN_EQUALS);
      _binary_expression(8, TOKEN_LEFT_SHIFT, TOKEN_RIGHT_SHIFT);
      _binary_expression(9, TOKEN_PLUS, TOKEN_MINUS);
      _binary_expression(10, TOKEN_ASTERISK, TOKEN_FORWARD_SLASH, TOKEN_MOD);
      default: expr = parser_primary_expression(p);
   }

   return expr;
}

node_t *parser_primary_expression(parser_t *p) {
   node_t *expr = NULL;

   if (lookahead(0).type == TOKEN_LEFT_PARENTHESES) {
      parser_eat(p, TOKEN_LEFT_PARENTHESES);

      expr = parser_expression(p, 0);

      parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   } else if (lookahead(0).type == TOKEN_IDENTIFIER) {
      if (M_COMPARE(lookahead(1).type, TOKEN_BANG, TOKEN_LEFT_PARENTHESES)) expr = parser_call_expression(p);
      else expr = parser_identifer(p);
   } else expr = parser_literal(p);

   return expr;
}

node_t *parser_unary_expression(parser_t *p) {
   node_t *ident;
   TOKEN_TYPE op;

   if (lookahead(0).type == TOKEN_IDENTIFIER) {
      ident = parser_identifer(p);
      op = parser_eat(p, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS).type;
   } else {
      op = parser_eat(p, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS).type;
      ident = parser_identifer(p);
   }

   return node_init(NODE_UNARY_EXPRESSION, .UNARY_EXPRESSION = {op, ident});
}

node_t *parser_call_expression(parser_t *p) {
   node_t *expr = node_init(NODE_CALL_EXPRESSION, .CALL_EXPRESSION = { parser_identifer(p), dy_init(node_t *) });

   if (lookahead(0).type == TOKEN_BANG) {
      parser_eat(p, TOKEN_BANG);

      expr->CALL_EXPRESSION.curried = true;
   }

   parser_eat(p, TOKEN_LEFT_PARENTHESES);

   expr->CALL_EXPRESSION.args = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_RIGHT_PARENTHESES, parser_statement, NULL);

   parser_eat(p, TOKEN_RIGHT_PARENTHESES);

   return expr;
}

/* -- 
   IF 
   -- */

node_t *parser_if(parser_t *p) {
   node_t *ifstmt = node_init(NODE_IF);

   parser_eat(p, TOKEN_IF);

   ifstmt->IF.cond     = parser_expression(p, 0);
   ifstmt->IF.truecase = parser_arrow_block(p);

   parser_skip_newlines(p);

   if (lookahead(0).type == TOKEN_ELSE) {
      parser_eat(p, TOKEN_ELSE);

      ifstmt->IF.falsecase = parser_statement(p);
   } else ifstmt->IF.falsecase = node_init(NODE_NONE);

   p->req_semi = false;

   return ifstmt;
}

/* --- 
   FOR 
   --- */

node_t *parser_for(parser_t *p) {
   node_t *forstmt = node_init(NODE_FOR);

   parser_eat(p, TOKEN_FOR);

   if (M_COMPARE(lookahead(0).type, TOKEN_ARROW, TOKEN_LEFT_BRACE)) {
      forstmt->FOR.init = node_init(NODE_NONE);
      forstmt->FOR.cond = node_init(NODE_NONE);
      forstmt->FOR.post = node_init(NODE_NONE);
   } else {
      node_t *expr;

      if (lookahead(0).type != TOKEN_SEMI_COLON) {
         expr = parser_statement(p);

         if (lookahead(0).type != TOKEN_SEMI_COLON) {
            forstmt->FOR.init = node_init(NODE_NONE);
            forstmt->FOR.cond = expr;
            forstmt->FOR.post = node_init(NODE_NONE);

            goto STMTPARSE;
         } else parser_eat(p, TOKEN_SEMI_COLON);
      } else {
         parser_eat(p, TOKEN_SEMI_COLON);

         expr = node_init(NODE_NONE);
      }

      forstmt->FOR.init = expr;

      if (lookahead(0).type == TOKEN_SEMI_COLON) {
         parser_eat(p, TOKEN_SEMI_COLON);
         forstmt->FOR.cond = node_init(NODE_NONE);
      } else {
         forstmt->FOR.cond = parser_statement(p);
         parser_eat(p, TOKEN_SEMI_COLON);
      }

      if (!M_COMPARE(lookahead(0).type, TOKEN_ARROW, TOKEN_RIGHT_BRACE))
         forstmt->FOR.post = parser_statement(p);
      else forstmt->FOR.post = node_init(NODE_NONE);
   }

STMTPARSE:
   forstmt->FOR.stmt = parser_arrow_block(p);

   return forstmt;
}

node_t *parser_break(parser_t *p) {
   parser_eat(p, TOKEN_BREAK);

   return node_init(NODE_BREAK);
}

node_t *parser_continue(parser_t *p) {
   parser_eat(p, TOKEN_CONTINUE);

   return node_init(NODE_CONTINUE);
}

/* ----------- 
   ARROW BLOCK 
   ----------- */

node_t *parser_arrow_block(parser_t *p) {
   if (lookahead(0).type == TOKEN_ARROW) {
      parser_eat(p, TOKEN_ARROW);

      parser_skip_newlines(p);

      return parser_statement(p);
   } else {
      parser_skip_newlines(p);

      return parser_block(p);
   }
}

/* ---- 
   TYPE 
   ---- */

node_t *parser_type(parser_t *p) {
   return node_init(NODE_DATA_TYPE, .DATA_TYPE = { _parser_type(p) });
}

node_t *parser_infer_type(parser_t *p) {
   return node_init(NODE_DATA_TYPE, .DATA_TYPE = { type_init(TYPE_NONE) });
}

type_t *_parser_type(parser_t *p) {
   switch (lookahead(0).type) {
      case TOKEN_CARET: return parser_ptr_type(p);
      case TOKEN_LEFT_BRACKET: return parser_array_type(p);
      case TOKEN_LEFT_PARENTHESES: return parser_function_type(p);
      default: return parser_base_type(p);
   }
}

type_t *parser_base_type(parser_t *p) {
   return type_init(TYPE_NONE, parser_identifer(p)->IDENTIFIER.value);
}

type_t *parser_ptr_type(parser_t *p) {
   parser_eat(p, TOKEN_CARET);

   return type_init(TYPE_PTR, .ptr_base = _parser_type(p));
}

type_t *parser_array_type(parser_t *p) {
   type_t *type = type_init(TYPE_ARRAY);

   parser_eat(p, TOKEN_LEFT_BRACKET);

   type->length = parser_eat(p, TOKEN_NUMBER).num;

   parser_eat(p, TOKEN_RIGHT_BRACKET);

   type->arr_base = _parser_type(p);

   return type;
}

type_t *parser_function_type(parser_t *p) {
   type_t *funct = type_init(TYPE_FUNCTION);

   funct->args = dy_init(struct { span_t name; struct type_t *type; });

   parser_eat(p, TOKEN_LEFT_PARENTHESES);

   while (lookahead(0).type != TOKEN_RIGHT_PARENTHESES) {
      struct { span_t name; struct type_t *type; } arg;

      arg.name = parser_identifer(p)->IDENTIFIER.value;

      parser_eat(p, TOKEN_COLON);

      arg.type = _parser_type(p);

      dy_push_unsafe(funct->args, arg);

      if (lookahead(0).type == TOKEN_COMMA) parser_eat(p, TOKEN_COMMA);
   }

   parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   parser_eat(p, TOKEN_COLON);

   if (M_COMPARE(lookahead(0).type, TOKEN_IDENTIFIER, TOKEN_CARET, TOKEN_LEFT_BRACKET, TOKEN_LEFT_PARENTHESES))
      funct->ret = _parser_type(p);
   else
      funct->ret = type_init(TYPE_BASE, .const_name = "void");

   return funct;
}

/* ------- 
   LITERAL 
   ------- */

node_t *parser_literal(parser_t *p) {
   token_t token = parser_eat(p, TOKEN_NUMBER, TOKEN_STRING);

   switch (token.type) {
      case TOKEN_NUMBER: return node_init(NODE_NUMBER_LITERAL, .NUMBER_LITERAL = { token.num });
      case TOKEN_STRING: return node_init(NODE_STRING_LITERAL, .STRING_LITERAL = { token.span });

      default: eprint("Error: expected literal!");
   }
}

/* ---------- 
   IDENTIFIER 
   ---------- */

node_t *parser_identifer(parser_t *p) {
   if (lookahead(0).type == TOKEN_UNDERSCORE) {
      parser_eat(p, TOKEN_UNDERSCORE);

      return node_init(NODE_NONE);
   } else return node_init(NODE_IDENTIFIER, .IDENTIFIER = { parser_eat(p, TOKEN_IDENTIFIER).span });
}

/* -------------------- 
   VARIABLE DECLARATION 
   -------------------- */

node_t *parser_variable_declaration(parser_t *p) {
   node_t *dec = node_init(NODE_VARIABLE_DECLARATION);
   token_t eq;

   dec->VARIABLE_DECLARATION.ident = parser_identifer(p);
   
   parser_eat(p, TOKEN_COLON);

   if (!M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON, TOKEN_UNDERSCORE)) {
      dec->VARIABLE_DECLARATION.type = parser_type(p);

      if (M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON)) {
         eq = parser_eat(p, TOKEN_EQUALS, TOKEN_COLON);

         dec->VARIABLE_DECLARATION.expr = parser_statement(p);
      } else dec->VARIABLE_DECLARATION.expr = node_init(NODE_NONE);
   } else {
      if (lookahead(0).type == TOKEN_UNDERSCORE) parser_eat(p, TOKEN_UNDERSCORE);

      eq = parser_eat(p, TOKEN_EQUALS, TOKEN_COLON);

      dec->VARIABLE_DECLARATION.type = parser_infer_type(p);

      dec->VARIABLE_DECLARATION.expr = parser_statement(p);
   }

   dec->VARIABLE_DECLARATION.immutable = eq.type == TOKEN_COLON;

   return dec;
}

node_t *underscore_func(parser_t *p) {
   parser_eat(p, TOKEN_UNDERSCORE);

   return parser_infer_type(p);
}

node_t *parser_multi_variable_declaration(parser_t *p) {
   node_t *decs = node_init(NODE_MULTI, .MULTI = { dy_init(node_t *) });
   bool inmutable = false;

   dynarr_t(node_t *) idents = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_NONE, parser_identifer, parser_identifer);
   dynarr_t(node_t *) types = NULL;
   dynarr_t(node_t *) exprs = NULL;

   parser_eat(p, TOKEN_COLON);

   if (!M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON))
      types = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_NONE, parser_type, underscore_func);

   if (M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON)) {
      inmutable = parser_eat(p, TOKEN_EQUALS, TOKEN_COLON).type == TOKEN_COLON;

      exprs = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_NONE, parser_statement, parser_identifer);
   }

   if ((types != NULL && dy_len(types) != dy_len(idents)) || (exprs != NULL && dy_len(exprs) != dy_len(idents)))
      eprint("Error: Invalid Variable Declaration!");

   for (int i = 0; i < dy_len(idents); i++) {
      node_t *dec = node_init(NODE_VARIABLE_DECLARATION);

      dec->VARIABLE_DECLARATION.ident     = dyi(idents)[i];
      dec->VARIABLE_DECLARATION.type      = types == NULL ? node_init(NODE_NONE) : dyi(types)[i];
      dec->VARIABLE_DECLARATION.expr      = exprs == NULL ? node_init(NODE_NONE) : dyi(exprs)[i];
      dec->VARIABLE_DECLARATION.immutable = inmutable;

      dy_push(decs->MULTI.nodes, dec);
   }

   dy_free(idents);
   if (types != NULL) dy_free(types);
   if (exprs != NULL) dy_free(exprs);

   return decs;
}

/* -------- 
   FUNCTION 
   -------- */

node_t *parser_function_declaration(parser_t *p) {
   node_t *funct = parser_type(p);

   if (lookahead(0).type == TOKEN_EQUALS) {
      parser_eat(p, TOKEN_EQUALS);

      parser_skip_newlines(p);

      return node_init(NODE_FUNCTION_DECLARATION, .FUNCTION_DECLARATION = { funct, parser_statement(p) });
   } else return funct;
}

/* ------ 
   RETURN 
   ------ */

node_t *parser_return(parser_t *p) {
   parser_eat(p, TOKEN_RETURN);

   return node_init(NODE_RETURN, .RETURN = { parser_expression(p, 0) });
}

/* ----- 
   BLOCK 
   ----- */

node_t *parser_block(parser_t *p) {
   parser_eat(p, TOKEN_LEFT_BRACE);

   node_t *block = node_init(NODE_BLOCK, .BLOCK = { dy_init(node_t *) });

   while (lookahead(0).type != TOKEN_RIGHT_BRACE) { 
      dy_push(block->BLOCK.stmts, parser_statement(p));

      if (lookahead(0).type == TOKEN_RIGHT_BRACE)
         break;

      parser_semi(p);
   }

   parser_eat(p, TOKEN_RIGHT_BRACE);

   return block;
}

/* ---- 
   UTIL 
   ---- */

void parser_skip_newlines(parser_t *p) {
   while (lookahead(0).type == TOKEN_NEWLINE) parser_eat(p, TOKEN_NEWLINE);
}

dynarr_t(node_t *) parser_sep_list(parser_t *p, TOKEN_TYPE sep, TOKEN_TYPE end) {
   dynarr_t(node_t *) arr = dy_init(node_t *);

   do {
      if (dy_len(arr)) parser_eat(p, sep);
      if (lookahead(0).type == end) break;

      node_t *elm;

      switch (lookahead(0).type) {
         case TOKEN_LEFT_PARENTHESES:
         case TOKEN_STRING:
         case TOKEN_NUMBER:
         case TOKEN_UNDERSCORE:
         case TOKEN_IDENTIFIER:
         case TOKEN_LEFT_BRACE:
            elm = parser_statement(p);
            break;
         default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
      }

      dy_push(arr, elm);
   } while (lookahead(0).type == sep);

   return arr;
}

dynarr_t(node_t *) parser_sep_list_func(parser_t *p, TOKEN_TYPE sep, TOKEN_TYPE end, node_t *(*v_func)(parser_t *p), node_t *(*u_func)(parser_t *p)) {
   dynarr_t(node_t *) arr = dy_init(node_t *);

   do {
      if (dy_len(arr)) parser_eat(p, sep);
      if (lookahead(0).type == end) break;

      node_t *elm;

      if (lookahead(0).type == TOKEN_UNDERSCORE) {
         if (u_func != NULL) elm = u_func(p);
         else {
            parser_eat(p, TOKEN_UNDERSCORE);
            continue;
         }
      } else elm = v_func(p);

      dy_push(arr, elm);
   } while (lookahead(0).type == sep);

   return arr;
}

token_t _parser_eat(parser_t *p, size_t n, ...) {
   va_list args;
   va_start(args, n);

   for (int i = 0; i < n; i++) {
      TOKEN_TYPE a = va_arg(args, TOKEN_TYPE);

      if (a == lookahead(0).type) {
         token_t ret = lookahead(0);
         lookahead(0) = lookahead(1);
         lookahead(1) = lexer_get_next_token(p->lexer);

         return ret;
      }
   }

   va_end(args);
   va_start(args, n);

   printf("Error: expected: ");

   for (int i = 0; i < n; i++) {
      if (i != 0) printf(" | ");

      print_token_type(va_arg(args, TOKEN_TYPE));
   }

   printf(" got: ");
   print_token_type(lookahead(0).type);
   printf("!\n");

   error_token(p->lexer, lookahead(0), "");

   exit(-1);
}

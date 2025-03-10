#include "parser.h"

/*
   FILE
      | VariableDeclaration
      | Impl
      | Empty
      ;

   Statement*
      : Expression
      | VariableDeclaration
      | Impl
      | Block
      | If
      | Return
      ;
   
   (Returns Value)
   Expression*
      : Literal
      | Identifier
      | BinaryExpression
      | UnaryExpression
      | Alias
      | FunctionDeclaration
      | Struct
      ;

   Alias
      : 'alias' Type
      ;

   Struct
      : 'struct' '{' (Identifier ':' Type)(,) '}'
      ;

   Impl
      : 'impl' Type '{' (VariableDeclaration(s)) '}'
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
      | '(' (Identifier(,)) Identifier ':' Types(,) ')' ':' (Type)
      ;

   BinaryExpression
      : Expression '+'|'-'|'*'|'/' ('=') Expression
      ;

   UnaryExpression*
      : SignExpression
      | CallExpression
      | IncDecExpression
      | NotExpression
      | SubscriptExpression
      | DerefExpression
      | AddrExpression
      | CastExpression
      ;

   CallExpression
      : Identifier ('!') '(' Expression(,) ')'
      ; 

   NotExpression
      : '!'|'~' Expression
      ;

   SignExpression
      : '+'|'-' Expression
      ;

   IncDecExpression
      : Identifier '++'|'--'
      | '++'|'--' Identifier
      ;

   SubscriptExpression
      : Identifier '[' Expression ']'
      ;

   DerefExpression
      : Expression '^'
      ;

   AddrExpression
      : '&' Expression
      ;

   CastExpression
      : '(' Type ')' Expression
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
      | NUMBER '.' NUMBER
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

node_t *parser_parse(size_t amt, FILE **files, type_handler_t *ty_hdl) {
   node_t *AST = node_init(NODE_PROJECT, .PROJECT = { dy_init(node_t *) });

   for (int i = 0; i < amt; i++)
      dy_push(AST->PROJECT.modules, parser_parse_file(files[i], ty_hdl));

   return AST;
}

node_t *parser_parse_file(FILE *f, type_handler_t *ty_hdl) {
   parser_t *inner = calloc(1, sizeof(parser_t));

   inner->lexer = lexer_new(f);
   inner->lookaheads[0] = lexer_get_next_token(inner->lexer);
   inner->lookaheads[1] = lexer_get_next_token(inner->lexer);
   inner->lookaheads[2] = lexer_get_next_token(inner->lexer);
   inner->lookaheads[3] = lexer_get_next_token(inner->lexer);
   inner->req_semi = true;
   inner->ty_hdl = ty_hdl;

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
   parser_eat(p, TOKEN_MODULE);

   node_t *file = node_init(NODE_FILE, .FILE = { 
      parser_eat(p, TOKEN_IDENTIFIER).str, 
      dy_init(wchar_t *), 
      dy_init(node_t *) 
   });

   p->module = file->FILE.name;

   while (!(M_COMPARE(lookahead(0).type, TOKEN_END))) { 
      if (lookahead(0).type == TOKEN_NONE)
         break;

      node_t *node;

      switch (lookahead(0).type) {
         case TOKEN_IMPORT:
            parser_eat(p, TOKEN_IMPORT);

            dy_push(file->FILE.imports, parser_eat(p, TOKEN_IDENTIFIER).str);

            parser_eat(p, TOKEN_NEWLINE);

            continue;
         case TOKEN_UNDERSCORE:
         case TOKEN_IDENTIFIER:
            switch (lookahead(1).type) {
               case TOKEN_COLON: node = parser_variable_declaration(p); goto SWITCHEND;
               case TOKEN_COMMA: node = parser_multi_variable_declaration(p); goto SWITCHEND;
               default: goto DEFAULT;
            }
            goto SWITCHEND;
         case TOKEN_IMPL:
            node = parser_impl(p);
            goto SWITCHEND;
         case TOKEN_SEMI_COLON:
         case TOKEN_NEWLINE:
            node = node_init(NODE_EMPTY);
            goto SWITCHEND;
DEFAULT:
         default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
      }

SWITCHEND:; // Why cant I declare things after labels and switches? ;-;
      dynarr_t(node_t *) scope = file->FILE.stmts;

      // switch (node->type == NODE_MULTI ? dyi(node->MULTI.nodes)[0]->type : node->type) {
      //    case NODE_NONE:
      //    case NODE_EMPTY:
      //    case NODE_VARIABLE_DECLARATION: scope = file->FILE.stmts; break;
      //    case NODE_IMPL: scope = file->FILE.impls; break;
      //    default: eprint("Invalid Scope!!!", node->type);
      // }

      if (node->type == NODE_MULTI) {
         for (int i = 0; i < dy_len(node->MULTI.nodes); i++)
            dy_push(scope, dyi(node->MULTI.nodes)[i]);

         dy_free(node->MULTI.nodes);
         free(node);
      } else dy_push(scope, node);

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
      case TOKEN_UNINIT:
      case TOKEN_STRUCT:
      case TOKEN_LEFT_PARENTHESES:
      case TOKEN_AND:
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_PLUS_PLUS:
      case TOKEN_MINUS_MINUS:
      case TOKEN_BANG:
      case TOKEN_TILDE:
      case TOKEN_STRING:
      case TOKEN_FLOAT:
      case TOKEN_HEX:
      case TOKEN_TRUE:
      case TOKEN_FALSE:
      case TOKEN_NUMBER: return parser_expression(p);
      case TOKEN_UNDERSCORE:
      case TOKEN_IDENTIFIER:
         switch (lookahead(1).type) {
            case TOKEN_COLON: return parser_variable_declaration(p);
            case TOKEN_COMMA: return parser_multi_variable_declaration(p);
            default:          return parser_expression(p);
         }
      case TOKEN_LEFT_BRACE: return parser_block(p);
      case TOKEN_IF: return parser_if(p);
      case TOKEN_DOR:
      case TOKEN_FOR: return parser_for(p);
      case TOKEN_BREAK: return parser_break(p);
      case TOKEN_CONTINUE: return parser_continue(p);
      case TOKEN_RETURN: return parser_return(p);
      case TOKEN_ALIAS: return parser_alias(p);
      case TOKEN_DEFER: return parser_defer(p);
      case TOKEN_SEMI_COLON:
      case TOKEN_NEWLINE: return node_init(NODE_EMPTY);
      default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
   }
}

// This is bad...
node_t *parser_parenthesized_statement(parser_t *p) {
   if (lookahead(1).type == TOKEN_RIGHT_PARENTHESES || 
       lookahead(2).type == TOKEN_COLON || 
       (lookahead(1).type == TOKEN_IDENTIFIER && (lookahead(2).type == TOKEN_COMMA || lookahead(3).type == TOKEN_COLON)))
      return parser_function_declaration(p);
   else return _parser_binary_expression(p, 0);
   // return parser_function_declaration(p);

   // int i = 0;

   // while (p->lexer->buf[p->lexer->cursor + i] != '(') i--;
   // while (p->lexer->buf[p->lexer->cursor + i] != ')') i++;

   // switch (p->lexer->buf[p->lexer->cursor + i + 1]) {
   //    case ':': return parser_function_declaration(p);
   //    default: return _parser_binary_expression(p, 0);
   // }
}

/* ---------- 
   EXPRESSION 
   ---------- */

node_t *parser_expression(parser_t *p) {
   switch (lookahead(0).type) {
      case TOKEN_LEFT_PARENTHESES: return parser_parenthesized_statement(p);
      case TOKEN_STRING:
      case TOKEN_FLOAT:
      case TOKEN_HEX:
      case TOKEN_NUMBER:
      case TOKEN_AND:
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_PLUS_PLUS:
      case TOKEN_MINUS_MINUS:
      case TOKEN_BANG:
      case TOKEN_TILDE:
      case TOKEN_UNDERSCORE:
      case TOKEN_TRUE:
      case TOKEN_FALSE:
      case TOKEN_IDENTIFIER: return _parser_binary_expression(p, 0);
      case TOKEN_ALIAS: return parser_alias(p);
      case TOKEN_STRUCT: return parser_struct(p);
      case TOKEN_UNINIT: return parser_uninit(p);
      case TOKEN_SEMI_COLON:
      case TOKEN_NEWLINE: return node_init(NODE_EMPTY);
      default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
   }
}

#define _binary_expression(precedence, ...)                                 \
   ({                                                                       \
      case precedence:                                                      \
         expr = _parser_binary_expression(p, precedence + 1);               \
         while (M_COMPARE(lookahead(0).type, __VA_ARGS__)) {                \
            expr = node_init(NODE_BINARY_EXPRESSION, .BINARY_EXPRESSION = { \
               parser_eat(p, __VA_ARGS__).type,                             \
               expr,                                                        \
               _parser_binary_expression(p, precedence + 1)                 \
            });                                                             \
         }                                                                  \
         break;                                                             \
   })                                                                       \

node_t *_parser_binary_expression(parser_t *p, int precedence) {
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
      default: expr = parser_addr_expression(p);
   }

   return expr;
}

node_t *parser_addr_expression(parser_t *p) {
   if (lookahead(0).type == TOKEN_AND) {
      parser_eat(p, TOKEN_AND);

      return node_init(NODE_ADDR_EXPRESSION, .ADDR_EXPRESSION = { parser_addr_expression(p) });
   } else return parser_deref_expression(p);
}

node_t *parser_deref_expression(parser_t *p) {
   node_t *expr = parser_cast_expression(p);

   while (lookahead(0).type == TOKEN_CARET) {
      parser_eat(p, TOKEN_CARET);

      expr = node_init(NODE_DEREF_EXPRESSION, .DEREF_EXPRESSION = { expr });
   }

   return expr;
}

node_t *parser_cast_expression(parser_t *p) {
   if (lookahead(0).type == TOKEN_LEFT_PARENTHESES && 
       (lookahead(1).type == TOKEN_CARET ||
       (lookahead(1).type == TOKEN_IDENTIFIER && 
       lookahead(2).type == TOKEN_RIGHT_PARENTHESES && 
       M_COMPARE(lookahead(3).type, TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER, TOKEN_FLOAT, TOKEN_HEX, TOKEN_LEFT_PARENTHESES, TOKEN_PLUS, TOKEN_MINUS, TOKEN_BANG, TOKEN_TILDE, TOKEN_CARET)))) {
      parser_eat(p, TOKEN_LEFT_PARENTHESES);

      node_t *type = parser_type(p);

      parser_eat(p, TOKEN_RIGHT_PARENTHESES);

      return node_init(NODE_CAST_EXPRESSION, .CAST_EXPRESSION = { type, parser_cast_expression(p) });
   } else return parser_not_expression(p);
}

node_t *parser_not_expression(parser_t *p) {
   if (M_COMPARE(lookahead(0).type, TOKEN_BANG, TOKEN_TILDE))
      return node_init(NODE_NOT_EXPRESSION, .NOT_EXPRESSION = { parser_eat(p, TOKEN_BANG, TOKEN_TILDE).type, parser_not_expression(p) });
   else return parser_sign_expression(p);
}

node_t *parser_sign_expression(parser_t *p) {
   if (M_COMPARE(lookahead(0).type, TOKEN_PLUS, TOKEN_MINUS))
      return node_init(NODE_SIGN_EXPRESSION, .SIGN_EXPRESSION = { parser_eat(p, TOKEN_PLUS, TOKEN_MINUS).type, parser_sign_expression(p) });
   else return parser_pre_incdec_expression(p);
}

node_t *parser_pre_incdec_expression(parser_t *p) {
   if (M_COMPARE(lookahead(0).type, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS))
      return node_init(NODE_INCDEC_EXPRESSION, .INCDEC_EXPRESSION = { parser_eat(p, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS).type, parser_pre_incdec_expression(p), true });
   else return parser_struct_literal(p);
}

node_t *parser_struct_literal(parser_t *p) {
   node_t *expr = parser_feild_expression(p);

   if (lookahead(0).type == TOKEN_APOSTROOHE && lookahead(1).type == TOKEN_LEFT_BRACE) {
      expr = node_init(NODE_STRUCT_LITERAL, .STRUCT_LITERAL = { expr, dy_init(wchar_t *), dy_init(node_t *) });

      parser_eat(p, TOKEN_APOSTROOHE);
      parser_eat(p, TOKEN_LEFT_BRACE);

      parser_skip_newlines(p);

      while (lookahead(0).type != TOKEN_RIGHT_BRACE) {
         parser_skip_newlines(p);

         if (lookahead(1).type == TOKEN_EQUALS) {
            dy_push(expr->STRUCT_LITERAL.idents, parser_identifer_str(p));

            parser_eat(p, TOKEN_EQUALS);
         } else dy_push(expr->STRUCT_LITERAL.idents, NULL);

         dy_push(expr->STRUCT_LITERAL.exprs, parser_expression(p));

         if (lookahead(0).type == TOKEN_COMMA) parser_eat(p, TOKEN_COMMA);

         parser_skip_newlines(p);
      }

      parser_eat(p, TOKEN_RIGHT_BRACE);
   }

   return expr;
}

node_t *parser_feild_expression(parser_t *p) {
   node_t *expr = parser_method_expression(p);

   while (lookahead(0).type == TOKEN_DOT) {
      parser_eat(p, TOKEN_DOT);

      expr = node_init(NODE_FEILD_EXPRESSION, .FEILD_EXPRESSION = {
         expr,
         parser_identifer_str(p),
         false
      });
   }

   return expr;
}

node_t *parser_method_expression(parser_t *p) {
   node_t *expr = parser_subscript_expression(p);

   while (M_COMPARE(lookahead(0).type, TOKEN_DOT) && 
          M_COMPARE(lookahead(2).type, TOKEN_BANG, TOKEN_LEFT_PARENTHESES)) {
      parser_eat(p, TOKEN_DOT, TOKEN_COMMA);

      expr = node_init(NODE_METHOD_EXPRESSION, .METHOD_EXPRESSION = { 
         expr,
         parser_identifer_str(p),
         dy_init(node_t *), 
      });

      if (lookahead(0).type == TOKEN_BANG) {
         parser_eat(p, TOKEN_BANG);

         expr->METHOD_EXPRESSION.curried = true;
      }

      parser_eat(p, TOKEN_LEFT_PARENTHESES);

      expr->METHOD_EXPRESSION.args = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_RIGHT_PARENTHESES, (node_t *(*)(parser_t *p))parser_expression, NULL);

      parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   }

   return expr;
}

node_t *parser_subscript_expression(parser_t *p) {
   node_t *expr = parser_call_expression(p);

   while (lookahead(0).type == TOKEN_LEFT_BRACKET) {
      parser_eat(p, TOKEN_LEFT_BRACKET);

      expr = node_init(NODE_SUBSCRIPT_EXPRESSION, .SUBSCRIPT_EXPRESSION = { expr, parser_expression(p) });

      parser_eat(p, TOKEN_RIGHT_BRACKET);
   }

   return expr;
}

node_t *parser_call_expression(parser_t *p) {
   node_t *expr = parser_post_incdec_expression(p);

   while (M_COMPARE(lookahead(0).type, TOKEN_BANG, TOKEN_LEFT_PARENTHESES)) {
      expr = node_init(NODE_CALL_EXPRESSION, .CALL_EXPRESSION = { expr, dy_init(node_t *) });

      if (lookahead(0).type == TOKEN_BANG) {
         parser_eat(p, TOKEN_BANG);

         expr->CALL_EXPRESSION.curried = true;
      }

      parser_eat(p, TOKEN_LEFT_PARENTHESES);

      expr->CALL_EXPRESSION.args = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_RIGHT_PARENTHESES, (node_t *(*)(parser_t *p))parser_expression, NULL);

      parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   }

   return expr;
}

node_t *parser_post_incdec_expression(parser_t *p) {
   node_t *expr = parser_path_expression(p);

   while (M_COMPARE(lookahead(0).type, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS))
      expr = node_init(NODE_INCDEC_EXPRESSION, .INCDEC_EXPRESSION = { parser_eat(p, TOKEN_PLUS_PLUS, TOKEN_MINUS_MINUS).type, expr, false });

   return expr;
}

node_t *parser_path_expression(parser_t *p) {
   node_t *expr = parser_primary_expression(p);

   if (lookahead(0).type == TOKEN_APOSTROOHE && lookahead(1).type != TOKEN_LEFT_BRACE) {
      expr = node_init(NODE_PATH_EXPRESSION, .PATH_EXPRESSION = { expr });

      parser_eat(p, TOKEN_APOSTROOHE);

      expr->PATH_EXPRESSION.expr = lookahead(1).type == TOKEN_LEFT_PARENTHESES ? parser_call_expression(p) : parser_identifer(p);
   }

   return expr;
}

node_t *parser_primary_expression(parser_t *p) {
   node_t *expr = NULL;

   if (lookahead(0).type == TOKEN_LEFT_PARENTHESES) {
      parser_eat(p, TOKEN_LEFT_PARENTHESES);

      expr = parser_expression(p);

      parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   } else if (lookahead(0).type == TOKEN_IDENTIFIER) {
      expr = parser_identifer(p);
   } else expr = parser_literal(p);

   return expr;
}

// node_t *parser_module_feild_expression(parser_t *p) {
//    node_t *expr = node_init(NODE_FEILD_EXPRESSION);

//    expr->FEILD_EXPRESSION.expr = parser_identifer(p);

//    parser_eat(p, TOKEN_DOT);

//    expr->FEILD_EXPRESSION.member = parser_identifer_str(p);

//    return expr;
// }

/* -- 
   IF 
   -- */

node_t *parser_if(parser_t *p) {
   node_t *ifstmt = node_init(NODE_IF);

   parser_eat(p, TOKEN_IF);

   ifstmt->IF.cond     = parser_expression(p);
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
   node_t *forstmt = node_init(NODE_FOR, .FOR = { .dor = parser_eat(p, TOKEN_FOR, TOKEN_DOR).type == TOKEN_DOR });

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
         forstmt->FOR.cond = parser_expression(p);
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

/* ----- 
   DEFER
   ----- */

node_t *parser_defer(parser_t *p) {
   parser_eat(p, TOKEN_DEFER);

   return node_init(NODE_DEFER, .DEFER = { parser_statement(p) });
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

node_t *parser_infer_type(parser_t *p) {
   return node_init(NODE_NONE);
   // return node_init(NODE_TYPE_IDX, .TYPE_IDX = { type_init(p->ty_hdl, (type_t) { TYPE_INFER }) });
}

node_t *parser_type(parser_t *p) {
   node_t *type = node_init(NODE_TYPE_BASE, .TYPE_BASE = { .res = -1 });

   switch (lookahead(0).type) {
      case TOKEN_CARET: type->TYPE_BASE.type = parser_ptr_type(p); break;
      case TOKEN_LEFT_BRACKET: type->TYPE_BASE.type = parser_array_type(p); break;
      case TOKEN_LEFT_PARENTHESES: type->TYPE_BASE.type = parser_function_type(p); break;
      case TOKEN_STRUCT: type->TYPE_BASE.type = parser_struct_type(p); break;
      default: type->TYPE_BASE.type = parser_base_type(p); break;
   }

   return type;
}

node_t *parser_base_type(parser_t *p) {
   wchar_t *fstr = parser_identifer_str(p);

   if (lookahead(0).type == TOKEN_APOSTROOHE) {
      parser_eat(p, TOKEN_APOSTROOHE);

      return node_init(NODE_TYPE_NAME, .TYPE_NAME = { parser_identifer_str(p), fstr });

   } else return node_init(NODE_TYPE_NAME, .TYPE_NAME = { fstr, wcsdup(p->module) });
}

node_t *parser_ptr_type(parser_t *p) {
   parser_eat(p, TOKEN_CARET);

   return node_init(NODE_TYPE_PTR, .TYPE_PTR = { parser_type(p) });
}

node_t *parser_array_type(parser_t *p) {
   ERROR("UNIMPLEMENTED!");
   // type_idx tidx = type_init(p->ty_hdl, (type_t) { TYPE_ARRAY });
   // type_t *type  = type_get(p->ty_hdl, tidx);

   // parser_eat(p, TOKEN_LEFT_BRACKET);

   // type->length = parser_eat(p, TOKEN_NUMBER).num;

   // parser_eat(p, TOKEN_RIGHT_BRACKET);

   // type->arr_base = _parser_type(p);

   // return tidx;
}

node_t *parser_function_type(parser_t *p) {
   node_t *funct = node_init(NODE_TYPE_FUNCTION, .TYPE_FUNCTION = {
      .arg_names = dy_init(wchar_t *),
      .arg_types = dy_init(node_t *),
   });

   parser_eat(p, TOKEN_LEFT_PARENTHESES);

   if (lookahead(0).type == TOKEN_IDENTIFIER && lookahead(1).type != TOKEN_COLON) {
      funct->TYPE_FUNCTION.self = parser_identifer_str(p);

      if (lookahead(0).type == TOKEN_COMMA) parser_eat(p, TOKEN_COMMA);
   }

   while (lookahead(0).type != TOKEN_RIGHT_PARENTHESES) {
      dy_push(funct->TYPE_FUNCTION.arg_names, parser_identifer_str(p));

      parser_eat(p, TOKEN_COLON);

      dy_push(funct->TYPE_FUNCTION.arg_types, parser_type(p));

      if (lookahead(0).type == TOKEN_COMMA) parser_eat(p, TOKEN_COMMA);
   }

   parser_eat(p, TOKEN_RIGHT_PARENTHESES);
   parser_eat(p, TOKEN_COLON);

   if (M_COMPARE(lookahead(0).type, TOKEN_IDENTIFIER, TOKEN_CARET, TOKEN_LEFT_BRACKET, TOKEN_LEFT_PARENTHESES))
      funct->TYPE_FUNCTION.ret = parser_type(p);
   else
      funct->TYPE_FUNCTION.ret = node_init(NODE_NONE);

   return funct;
}

node_t *parser_struct_type(parser_t *p) {
   parser_eat(p, TOKEN_STRUCT);
   parser_eat(p, TOKEN_LEFT_BRACE);

   node_t *strt = node_init(NODE_TYPE_STRUCT, .TYPE_STRUCT = {
      dy_init(wchar_t *),
      dy_init(node_t *),
   });

   while (lookahead(0).type != TOKEN_RIGHT_BRACE) {
      parser_skip_newlines(p);

      dy_push(strt->TYPE_STRUCT.feild_names, parser_identifer_str(p));

      parser_eat(p, TOKEN_COLON);

      dy_push(strt->TYPE_STRUCT.feild_types, parser_type(p));

      if (lookahead(0).type == TOKEN_SEMI_COLON) parser_eat(p, TOKEN_SEMI_COLON);

      parser_skip_newlines(p);
   }

   parser_eat(p, TOKEN_RIGHT_BRACE);

   return strt;
}

/* ------ 
   STRUCT 
   ------ */

node_t *parser_struct(parser_t *p) {
   return node_init(NODE_STRUCT, .STRUCT = { parser_type(p) });
}

/* ---- 
   IMPL
   ---- */

node_t *parser_impl(parser_t *p) {
   node_t *stmt = node_init(NODE_IMPL);

   parser_eat(p, TOKEN_IMPL);

   stmt->IMPL.type  = parser_type(p);
   stmt->IMPL.funcs = dy_init(node_t *);

   parser_eat(p, TOKEN_LEFT_BRACE);
   parser_skip_newlines(p);

   while (lookahead(0).type != TOKEN_RIGHT_BRACE) {
      dy_push(stmt->IMPL.funcs, parser_variable_declaration(p));

      parser_skip_newlines(p);
   }

   parser_skip_newlines(p);

   parser_eat(p, TOKEN_RIGHT_BRACE);

   return stmt;
}

/* ------ 
   UNINIT 
   ------ */

node_t *parser_uninit(parser_t *p) {
   parser_eat(p, TOKEN_UNINIT);

   return node_init(NODE_UNINIT);
}

/* ------- 
   LITERAL 
   ------- */

node_t *parser_literal(parser_t *p) {
   // token_t token = parser_eat(p, TOKEN_NUMBER, TOKEN_FLOAT, TOKEN_HEX, TOKEN_STRING, TOKEN_IDENTIFIER);

   switch (lookahead(0).type) {
      case TOKEN_HEX:
      case TOKEN_NUMBER: return node_init(NODE_NUMBER_LITERAL, .NUMBER_LITERAL = { parser_eat(p, TOKEN_NUMBER, TOKEN_HEX).num });
      case TOKEN_TRUE:
      case TOKEN_FALSE: return node_init(NODE_BOOL_LITERAL, .BOOL_LITERAL = { parser_eat(p, TOKEN_TRUE, TOKEN_FALSE).type == TOKEN_TRUE });
      case TOKEN_STRING: return node_init(NODE_STRING_LITERAL, .STRING_LITERAL = { parser_eat(p, TOKEN_STRING).str });
      case TOKEN_FLOAT:;
         token_t tok = parser_eat(p, TOKEN_FLOAT);
         return node_init(NODE_FLOAT_LITERAL, .FLOAT_LITERAL = { tok.integer, tok.fraction });
      case TOKEN_IDENTIFIER: return parser_type(p);
      default: error_token(p->lexer, lookahead(0), "Error: Unexpected Token!");
   }
}

/* ---------- 
   IDENTIFIER 
   ---------- */

node_t *parser_identifer(parser_t *p) {
   if (lookahead(0).type == TOKEN_UNDERSCORE) {
      parser_eat(p, TOKEN_UNDERSCORE);

      return node_init(NODE_NONE);
   } else {
      // TODO: add current scope to parser and add it to all identifiers 
      return node_init(NODE_IDENTIFIER, .IDENTIFIER = { parser_eat(p, TOKEN_IDENTIFIER).str });
   }
}

wchar_t *parser_identifer_str(parser_t *p) {
   return parser_eat(p, TOKEN_IDENTIFIER).str;
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

         dec->VARIABLE_DECLARATION.expr = parser_expression(p);
      } else dec->VARIABLE_DECLARATION.expr = node_init(NODE_NONE);
   } else {
      if (lookahead(0).type == TOKEN_UNDERSCORE) parser_eat(p, TOKEN_UNDERSCORE);

      eq = parser_eat(p, TOKEN_EQUALS, TOKEN_COLON);

      dec->VARIABLE_DECLARATION.type = parser_infer_type(p);

      dec->VARIABLE_DECLARATION.expr = parser_expression(p);
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

   token_t col = parser_eat(p, TOKEN_COLON);

   if (!M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON))
      types = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_NONE, parser_type, underscore_func);

   if (M_COMPARE(lookahead(0).type, TOKEN_EQUALS, TOKEN_COLON)) {
      inmutable = parser_eat(p, TOKEN_EQUALS, TOKEN_COLON).type == TOKEN_COLON;

      exprs = parser_sep_list_func(p, TOKEN_COMMA, TOKEN_NONE, parser_expression, parser_identifer);
   }

   if ((types != NULL && dy_len(types) != dy_len(idents)) || (exprs != NULL && dy_len(exprs) != dy_len(idents)))
      error_token(p->lexer, col, "Error: Invalid Variable Declaration!");

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

/* ----- 
   ALIAS 
   ----- */

node_t *parser_alias(parser_t *p) {
   parser_eat(p, TOKEN_ALIAS);

   return node_init(NODE_ALIAS, .ALIAS = { parser_type(p) });
}

/* ------ 
   RETURN 
   ------ */

node_t *parser_return(parser_t *p) {
   parser_eat(p, TOKEN_RETURN);

   return node_init(NODE_RETURN, .RETURN = { 
      M_COMPARE(lookahead(0).type, TOKEN_NEWLINE, TOKEN_SEMI_COLON) ? node_init(NODE_NONE) : parser_expression(p) 
   });
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
         token_t ret  = lookahead(0);
         lookahead(0) = lookahead(1);
         lookahead(1) = lookahead(2);
         lookahead(2) = lookahead(3);
         lookahead(3) = lexer_get_next_token(p->lexer);

         return ret;
      }
   }

   va_end(args);
   va_start(args, n);

   // printf("Error: expected: ");

   // for (int i = 0; i < n; i++) {
   //    if (i != 0) printf(" | ");

   //    print_token_type(va_arg(args, TOKEN_TYPE));
   // }

   // printf(" got: ");
   // print_token_type(lookahead(0).type);
   // printf("!\n");

   error_token(p->lexer, lookahead(0), "Error: expected %s got %s", token_type_str(va_arg(args, TOKEN_TYPE)), token_type_str(lookahead(0).type));

   exit(-1);
}

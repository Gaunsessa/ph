#include "cgen.h"

#define dy_push_str(arr, str) ({ for (int _i_ = 0; (str)[_i_] != '\0'; _i_++) dy_push((arr), (str)[_i_]); })
#define dy_push_ptr(arr, ptr, len) ({ for (int _i_ = 0; _i_ < len; _i_++) dy_push((arr), (ptr)[_i_]); })

wchar_t *cgen_generate(node_t *AST) {
   if (AST->type != NODE_PROJECT) eprint("Error: Invalid AST!");

   buf_t buffer = dy_init(wchar_t);

   cgen_file(buffer, dyi(AST->PROJECT.modules)[0]);
   
   dy_push(buffer, 0);

   return dyi(buffer);
}

void cgen_file(buf_t buf, node_t *file) {
   // dy_push_str(buf, "#include <stdio.h>\n");
   // dy_push_str(buf, "#include <stdlib.h>\n");
   dy_push_str(buf, L"#include <stdint.h>\n");
   dy_push_str(buf, L"#include <stddef.h>\n");

   for (int i = 0; i < dy_len(file->FILE.stmts); i++) {
      node_t *stmt = dyi(file->FILE.stmts)[i];

      switch (stmt->type) {
         case NODE_VARIABLE_DECLARATION:
            cgen_variable_declaration(buf, stmt);
            break;
         case NODE_IMPL:
            for (int i = 0; i < dy_len(stmt->IMPL.funcs); i++)
               cgen_variable_declaration(buf, dyi(stmt->IMPL.funcs)[i]);

            break;
         case NODE_EMPTY: break;
         default: eprint("WHAT????");
      }

      if (stmt->type != NODE_EMPTY) dy_push(buf, L';');
   }

   dy_push_str(buf, L"int main() { return main_(); }");
}

void cgen_statement(buf_t buf, node_t *stmt) {
   switch (stmt->type) {
      case NODE_NUMBER_LITERAL:
      case NODE_STRING_LITERAL:
      case NODE_BINARY_EXPRESSION:
      case NODE_CALL_EXPRESSION:
      case NODE_FEILD_EXPRESSION:
      case NODE_METHOD_EXPRESSION:
         return cgen_expression(buf, stmt);
      case NODE_VARIABLE_DECLARATION:
         return cgen_variable_declaration(buf, stmt);
      case NODE_BLOCK:
         return cgen_block(buf, stmt);
      case NODE_IF:
         return cgen_if(buf, stmt);
      case NODE_FOR:
         return cgen_for(buf, stmt);
      case NODE_BREAK:
         return cgen_break(buf, stmt);
      case NODE_CONTINUE:
         return cgen_continue(buf, stmt);
      case NODE_RETURN:
         return cgen_return(buf, stmt);
      case NODE_EMPTY:
         break;
      default: ERROR("Unimplemented statment!", stmt->type);
   }
}

void cgen_expression(buf_t buf, node_t *expr) {
   dy_push(buf, L'(');

   switch (expr->type) {
      case NODE_BINARY_EXPRESSION:
         cgen_expression(buf, expr->BINARY_EXPRESSION.left);

         dy_push_str(buf, ht_get_vs(TOKEN_STRS, expr->BINARY_EXPRESSION.op));

         cgen_expression(buf, expr->BINARY_EXPRESSION.right);

         break;
      case NODE_CALL_EXPRESSION:
         dy_push_str(buf, expr->CALL_EXPRESSION.func->IDENTIFIER.value);

         dy_push(buf, L'(');

         for (int i = 0; i < dy_len(expr->CALL_EXPRESSION.args); i++) {
            if (i != 0) dy_push(buf, L',');

            cgen_expression(buf, dyi(expr->CALL_EXPRESSION.args)[i]);
         }

         dy_push(buf, L')');
         break;
      case NODE_ADDR_EXPRESSION:
         dy_push(buf, L'&');

         cgen_expression(buf, expr->ADDR_EXPRESSION.expr);

         break;
      case NODE_DEREF_EXPRESSION:
         dy_push(buf, L'*');

         cgen_expression(buf, expr->DEREF_EXPRESSION.expr);

         break;
      case NODE_CAST_EXPRESSION:
         dy_push(buf, L'(');

         cgen_type(buf, expr->CAST_EXPRESSION.type);

         dy_push(buf, L')');

         cgen_expression(buf, expr->CAST_EXPRESSION.expr);
         break;
      case NODE_FEILD_EXPRESSION:
         cgen_feild(buf, expr);
         break;
      case NODE_IDENTIFIER:
         cgen_identifier(buf, expr);
         break;
      case NODE_SIGN_EXPRESSION:
         dy_push_str(buf, ht_get_vs(TOKEN_STRS, expr->SIGN_EXPRESSION.op));
         cgen_expression(buf, expr->SIGN_EXPRESSION.expr);
         break;
      case NODE_STRUCT_LITERAL:
         dy_push(buf, L'(');

         cgen_type(buf, expr->STRUCT_LITERAL.type);

         dy_push_str(buf, L"){");

         for (int i = 0; i < dy_len(expr->STRUCT_LITERAL.exprs); i++) {
            if (i != 0) dy_push(buf, L',');

            wchar_t *ident = dyi(expr->STRUCT_LITERAL.idents)[i];
            node_t *exp    = dyi(expr->STRUCT_LITERAL.exprs)[i];

            if (ident == NULL) cgen_expression(buf, exp);
            else {
               dy_push(buf, L'.');

               dy_push_str(buf, ident);

               dy_push(buf, L'=');

               cgen_expression(buf, exp);
            }
         }

         dy_push(buf, L'}');

         break;
      case NODE_METHOD_EXPRESSION:
         dy_push_str(buf, expr->METHOD_EXPRESSION.member);

         dy_push(buf, L'(');

         if (!expr->METHOD_EXPRESSION.ptr) dy_push(buf, L'&');
         cgen_expression(buf, expr->METHOD_EXPRESSION.expr);

         if (dy_len(expr->METHOD_EXPRESSION.args) > 0) dy_push(buf, L',');

         for (int i = 0; i < dy_len(expr->METHOD_EXPRESSION.args); i++) {
            if (i != 0) dy_push(buf, L',');

            cgen_expression(buf, dyi(expr->METHOD_EXPRESSION.args)[i]);
         }

         dy_push(buf, L')');

         break;
      case NODE_NUMBER_LITERAL:
      case NODE_FLOAT_LITERAL:
      case NODE_STRING_LITERAL:
         cgen_literal(buf, expr);
         break;
      default: ERROR("Unknown expression!", expr->type);
   }

   dy_push(buf, L')');
}

void cgen_if(buf_t buf, node_t *ifstmt) {
   node_def(ifstmt, IF);

   dy_push_str(buf, L"if(");

   cgen_expression(buf, node->cond);

   dy_push(buf, L')');

   if (node->truecase->type == NODE_BLOCK) cgen_block(buf, node->truecase);
   else {
      dy_push(buf, L'{');

      cgen_statement(buf, node->truecase);

      dy_push_str(buf, L";}");
   }

   if (node->falsecase->type != NODE_NONE) {
      dy_push_str(buf, L"else ");

      cgen_statement(buf, node->falsecase);
   }
}

void cgen_for(buf_t buf, node_t *forstmt) {
   node_def(forstmt, FOR);

   if (node->dor) {
      dy_push(buf, L'{');

      if (node->init->type != NODE_NONE)
         cgen_statement(buf, node->init);

      dy_push_str(buf, L";do{");

      if (node->stmt->type == NODE_BLOCK) cgen_block(buf, node->stmt);
      else {
         dy_push(buf, L'{');

         cgen_statement(buf, node->stmt);

         dy_push_str(buf, L";} ");
      }

      if (node->post->type != NODE_NONE) {
         cgen_statement(buf, node->post);
         dy_push(buf, L';');
      }

      dy_push_str(buf, L"}while(");

      if (node->cond->type != NODE_NONE)
         cgen_statement(buf, node->cond);
      else dy_push(buf, L'1');

      dy_push_str(buf, L");}");
   } else {
      dy_push_str(buf, L"for(");

      if (node->init->type != NODE_NONE) 
         cgen_statement(buf, node->init);

      dy_push(buf, L';');

      if (node->cond->type != NODE_NONE) 
         cgen_statement(buf, node->cond);

      dy_push(buf, L';');

      if (node->post->type != NODE_NONE) 
         cgen_statement(buf, node->post);

      dy_push(buf, L')');

      if (node->stmt->type == NODE_BLOCK) cgen_block(buf, node->stmt);
      else {
         dy_push(buf, L'{');

         cgen_statement(buf, node->stmt);

         dy_push_str(buf, L";} ");
      }
   }
}

void cgen_break(buf_t buf, node_t *brk) {
   ASSERT(brk->type, NODE_BREAK);

   dy_push_str(buf, L"break");
}

void cgen_continue(buf_t buf, node_t *cnt) {
   ASSERT(cnt->type, NODE_CONTINUE);

   dy_push_str(buf, L"continue");
}

void cgen_return(buf_t buf, node_t *ret) {
   node_def(ret, RETURN);

   dy_push_str(buf, L"return ");

   if (node->value->type != NODE_NONE) cgen_expression(buf, node->value);
}

void cgen_feild(buf_t buf, node_t *acc) {
   node_def(acc, FEILD_EXPRESSION);

   // if (node->member->type == NODE_CALL_EXPRESSION) {
   //    dy_push_str(buf, node->member->CALL_EXPRESSION.func->IDENTIFIER.value);

   //       dy_push(buf, L'(');

   //       if (!node->ptr) dy_push(buf, L'&');
   //       cgen_expression(buf, node->expr);

   //       if (dy_len(node->member->CALL_EXPRESSION.args)) dy_push(buf, L',');

   //       for (int i = 0; i < dy_len(node->member->CALL_EXPRESSION.args); i++) {
   //          if (i != 0) dy_push(buf, L',');

   //          cgen_expression(buf, dyi(node->member->CALL_EXPRESSION.args)[i]);
   //       }

   //       dy_push(buf, L')');
   // } else {
      cgen_expression(buf, node->expr);

      dy_push_str(buf, node->ptr ? L"->" : L".");

      dy_push_str(buf, node->member);
   // }
}

void cgen_variable_declaration(buf_t buf, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   if (node->expr->type == NODE_FUNCTION_DECLARATION) {
      node_t *funct = node->expr->FUNCTION_DECLARATION.type;

      _cgen_type(buf, funct->DATA_TYPE.type->ret);

      dy_push(buf, L' ');

      cgen_identifier(buf, node->ident);

      dy_push(buf, L'(');

      if (funct->DATA_TYPE.type->self.name != NULL) {
         _cgen_type(buf, funct->DATA_TYPE.type->self.type);

         dy_push(buf, L' ');

         dy_push_str(buf, funct->DATA_TYPE.type->self.name);

         if (!(dy_len(funct->DATA_TYPE.type->args) - 1)) dy_push(buf, L',');
      }

      for (int i = 0; i < dy_len(funct->DATA_TYPE.type->args); i++) {
         if (i != 0) dy_push(buf, L',');

         _cgen_type(buf, dyi(funct->DATA_TYPE.type->args)[i].type);

         dy_push(buf, L' ');

         dy_push_str(buf, dyi(funct->DATA_TYPE.type->args)[i].name);
      }

      dy_push(buf, L')');

      if (node->expr->FUNCTION_DECLARATION.stmt->type != NODE_UNINIT) {
         if (node->expr->FUNCTION_DECLARATION.stmt->type == NODE_BLOCK)
            cgen_block(buf, node->expr->FUNCTION_DECLARATION.stmt);
         else {
            dy_push(buf, L'{');

            cgen_statement(buf, node->expr->FUNCTION_DECLARATION.stmt);

            dy_push_str(buf, L";}");
         }
      }
   } else if (node->expr->type == NODE_ALIAS) {
      dy_push_str(buf, L"typedef ");

      cgen_type(buf, node->expr->ALIAS.type);

      dy_push(buf, L' ');

      cgen_identifier(buf, node->ident);
   } else if (node->expr->type == NODE_STRUCT) {
      dy_push_str(buf, L"typedef struct {");

      for (int i = 0; i < dy_len(node->expr->STRUCT.type->DATA_TYPE.type->feilds); i++) {
         struct { wchar_t *name; type_t *type; } *feild = (void *)&dyi(node->expr->STRUCT.type->DATA_TYPE.type->feilds)[i];

         _cgen_type(buf, feild->type);

         dy_push(buf, L' ');

         dy_push_str(buf, feild->name);

         dy_push(buf, L';');
      }

      dy_push_str(buf, L"} ");

      cgen_identifier(buf, node->ident);
   } else {
      cgen_type(buf, node->type);

      dy_push(buf, L' ');

      cgen_identifier(buf, node->ident);

      if (node->expr->type != NODE_UNINIT) {
         dy_push(buf, L'=');

         if (node->expr->type != NODE_NONE) {
            cgen_expression(buf, node->expr);
         } else {
            dy_push_str(buf, L"{0}");
         }
      }
   }
}

void cgen_block(buf_t buf, node_t *block) {
   node_def(block, BLOCK);

   dy_push(buf, L'{');

   for (int i = 0; i < dy_len(node->stmts); i++) {
      cgen_statement(buf, dyi(node->stmts)[i]);

      if (dyi(node->stmts)[i]->type != NODE_EMPTY) dy_push(buf, L';');
   }

   dy_push(buf, L'}');
}

void cgen_type(buf_t buf, node_t *type) {
   if (type->type == NODE_DATA_TYPE) _cgen_type(buf, type->DATA_TYPE.type);
   else cgen_expression(buf, type);
   // node_def(type, DATA_TYPE);
}

void _cgen_type(buf_t buf, type_t *type) {
   switch (type->type) {
      case TYPE_NONE:
         ERROR("Tried to Generate None Type!");
         break;
      case TYPE_BASE:
         dy_push_str(buf, type_base_cname(type));
         break;
      case TYPE_ALIAS:
         dy_push_str(buf, type->name);
         break;
      case TYPE_PTR:
         _cgen_type(buf, type->ptr_base);
         dy_push(buf, L'*');
         break;
      case TYPE_STRUCT:
         dy_push_str(buf, L"struct ");
         break;
      default: ERROR("Unimplemented Type!", type->type);
   }
}

void cgen_identifier(buf_t buf, node_t *ident) {
   node_def(ident, IDENTIFIER);

   dy_push_str(buf, node->value);
}

void cgen_literal(buf_t buf, node_t *lit) {
   ASSERT(lit->type, NODE_NUMBER_LITERAL, NODE_FLOAT_LITERAL, NODE_STRING_LITERAL);

   switch (lit->type) {
      case NODE_FLOAT_LITERAL: {
         char str[snprintf(NULL, 0, "%lu.%lu", lit->FLOAT_LITERAL.integer, lit->FLOAT_LITERAL.fraction) + 1];
         sprintf(str, "%lu.%lu", lit->FLOAT_LITERAL.integer, lit->FLOAT_LITERAL.fraction);

         dy_push_str(buf, str);
      } break;
      case NODE_NUMBER_LITERAL: {
         char str[snprintf(NULL, 0, "%lu", lit->NUMBER_LITERAL.value) + 1];
         sprintf(str, "%lu", lit->NUMBER_LITERAL.value);

         dy_push_str(buf, str);
      } break;
      case NODE_STRING_LITERAL:
         dy_push(buf, L'"');
         dy_push_str(buf, lit->STRING_LITERAL.span);
         dy_push(buf, L'"');
         break;
      default: unreachable();
   }
}
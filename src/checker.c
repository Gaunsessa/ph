#include "checker.h"

#define error(msg) ({ { ckr->errors++; printf("%s\n", msg); return NULL; } })

bool checker_check(node_t *AST) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->scope = (scope_t) {
      ht_init_sv(type_t *),
      NULL,
      NULL,
   };

   ckr->cur_scope = &ckr->scope;
   ckr->error = false;

   checker_file(ckr, AST);

   return false;
}

type_t *checker_file(checker_t *ckr, node_t *file) {
   node_def(file, FILE);

   for (int i = 0; i < dy_len(node->stmts); i++) {
      node_t *stmt = dyi(node->stmts)[i];

      if (stmt->type != NODE_VARIABLE_DECLARATION) continue;

      if (stmt->VARIABLE_DECLARATION.expr->type == NODE_FUNCTION_DECLARATION) {
         ht_set_sv(ckr->scope.decls, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value, stmt->VARIABLE_DECLARATION.expr->FUNCTION_DECLARATION.type->DATA_TYPE.type);
      } else ht_set_sv(ckr->scope.decls, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value, checker_variable_declaration(ckr, stmt));
   }

   ckr->error = true;

   for (int i = 0; i < dy_len(node->stmts); i++) {
      node_t *stmt = dyi(node->stmts)[i];

      if (stmt->type == NODE_VARIABLE_DECLARATION) {
         if (stmt->VARIABLE_DECLARATION.expr->type == NODE_FUNCTION_DECLARATION) 
            checker_function_declaration(ckr, stmt->VARIABLE_DECLARATION.expr);
      }
   }
}

type_t *checker_statement(checker_t *ckr, node_t *stmt) {
   switch (stmt->type) {
      case NODE_VARIABLE_DECLARATION: return checker_variable_declaration(ckr, stmt);
      case NODE_FUNCTION_DECLARATION: return checker_function_declaration(ckr, stmt);
      case NODE_BLOCK: return checker_block(ckr, stmt);
      case NODE_BINARY_EXPRESSION:
      case NODE_UNARY_EXPRESSION:
      case NODE_CALL_EXPRESSION:
      case NODE_IDENTIFIER:
      case NODE_STRING_LITERAL:
      case NODE_NUMBER_LITERAL: return checker_expression(ckr, stmt);
      case NODE_RETURN: return checker_return(ckr, stmt);
      case NODE_ALIAS: return checker_alias(ckr, stmt);
      case NODE_EMPTY: return NULL;
      default: eprint("Not Implemented Statement!", stmt->type);
   }
}

type_t *checker_block(checker_t *ckr, node_t *block) {
   node_def(block, BLOCK);

   scope_t *prev_scope = ckr->cur_scope;

   scope_t scope = {
      ht_init_sv(type_t *),
      NULL,
      ckr->cur_scope,
   };

   ckr->cur_scope = &scope;

   for (int i = 0; i < dy_len(node->stmts); i++) {
      checker_statement(ckr, dyi(node->stmts)[i]);
   }

   ht_free(scope.decls);
   ckr->cur_scope = prev_scope;
}

type_t *checker_expression(checker_t *ckr, node_t *expr) {
   switch (expr->type) {
      case NODE_CALL_EXPRESSION: return checker_call_expression(ckr, expr);
      case NODE_STRING_LITERAL:
      case NODE_NUMBER_LITERAL: return checker_literal(ckr, expr);
      case NODE_IDENTIFIER: return checker_identifier(ckr, expr);
   }
}

type_t *checker_binary_expression(checker_t *ckr, node_t *expr);

type_t *checker_unary_expression(checker_t *ckr, node_t *expr);

type_t *checker_call_expression(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *funct = checker_identifier(ckr, node->func);
   if (funct == NULL) return NULL;

   if (dy_len(funct->args) != dy_len(node->args))
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_statement(ckr, dyi(node->args)[i]);
      if (argt == NULL) return NULL;

      if (!type_cmp(checker_reslove_type(ckr, dyi(funct->args)[i].type), argt))
         error("Invalid Argument Type Provided!");
   }

   return funct->ret;
}

type_t *checker_literal(checker_t *ckr, node_t *lit) {
   switch (lit->type) {
      case NODE_STRING_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_STRING);
      case NODE_NUMBER_LITERAL: return ht_get(BASE_TYPE_ENUM_VALUES, BASE_INT);
      default: eprint("Invalid Literal Type!");
   }
}

type_t *checker_identifier(checker_t *ckr, node_t *ident) {
   node_def(ident, IDENTIFIER);

   type_t *type = checker_get_type(ckr, node->value);

   if (type == NULL) error("Unknown Identifier!");

   return type;
}

type_t *checker_variable_declaration(checker_t *ckr, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   char *name    = node->ident->IDENTIFIER.value;
   type_t *type  = checker_reslove_type(ckr, node->type->DATA_TYPE.type); 
   type_t *infer = node->expr->type != NODE_NONE ? checker_statement(ckr, node->expr) : NULL;

   if (infer != NULL) {
      if (type->type != TYPE_INFER) {
         if (!type_cmp(type, infer)) error("Variable Declation Types Dont Match!");
      } else {
         type = infer;
         vard->VARIABLE_DECLARATION.type->DATA_TYPE.type = infer;
      }
   }

   if (!checker_can_reslove_type(ckr, type)) error("Cannot Resolve Type!");

   if (ht_exists_sv(ckr->cur_scope->decls, name)) error("Identifier Already Defined!");

   ht_set_sv(ckr->cur_scope->decls, name, type);

   return type;
}

type_t *checker_function_declaration(checker_t *ckr, node_t *funcd) {
   node_def(funcd, FUNCTION_DECLARATION);

   scope_t *prev_scope = ckr->cur_scope;

   scope_t scope = {
      ht_init_sv(type_t *),
      NULL,
      ckr->cur_scope,
   };

   ckr->cur_scope = &scope;

   for (int i = 0; i < dy_len(node->type->DATA_TYPE.type->args); i++) {
      char *name   = dyi(node->type->DATA_TYPE.type->args)[i].name;
      type_t *type = checker_reslove_type(ckr, dyi(node->type->DATA_TYPE.type->args)[i].type);

      ht_set_sv(ckr->cur_scope->decls, name, type);
   }

   checker_statement(ckr, node->stmt);

   ht_free(scope.decls);
   ckr->cur_scope = prev_scope;

   return checker_reslove_type(ckr, node->type->DATA_TYPE.type);
}

type_t *checker_if(checker_t *ckr, node_t *ifstmt);
type_t *checker_for(checker_t *ckr, node_t *forstmt);

type_t *checker_return(checker_t *ckr, node_t *ret) {
   node_def(ret, RETURN);


}

type_t *checker_alias(checker_t *ckr, node_t *alias) {
   node_def(alias, ALIAS);

   if (!checker_can_reslove_type(ckr, node->type->DATA_TYPE.type))
      error("Cannot Resolve Type!");

   return checker_reslove_type(ckr, node->type->DATA_TYPE.type);
}

bool checker_can_reslove_type(checker_t *ckr, type_t *type) {
   switch (type->type) {
      case TYPE_NONE: return false;
      case TYPE_INFER: return false;
      case TYPE_BASE: return true;
      case TYPE_ALIAS: return checker_get_type(ckr, type->name) != NULL;
      case TYPE_PTR: return checker_can_reslove_type(ckr, type->ptr_base);
      case TYPE_ARRAY: return checker_can_reslove_type(ckr, type->arr_base);
      case TYPE_FUNCTION: return true;
      case TYPE_STRUCT: return true;
   }
}

type_t *checker_reslove_type(checker_t *ckr, type_t *type) {
   return type->type == TYPE_ALIAS ? checker_reslove_type(ckr, checker_get_type(ckr, type->name)) : type;
}

type_t *checker_get_type(checker_t *ckr, char *ident) {
   scope_t *cur = ckr->cur_scope;

   while (cur != NULL) {
      if (ht_exists_sv(cur->decls, ident))
         return ht_get_sv(cur->decls, ident);

      cur = cur->parent;
   }

   return NULL;
}
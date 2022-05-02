#include "checker.h"

#define error(msg) ({ printf("%s\n", msg); return false; })

#define _se_check(base, ckr, node) ({ bool s = base(ckr, node); bool e = base##_end(ckr, node); s || e; }) 

void _check_start(node_t *node) {
   checker_check_start(node, NULL);
}

void _check_end(node_t *node) {
   checker_check_end(node, NULL);
}

bool checker_check(node_t *AST) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->scope = (scope_t) {
      ht_init_sv(type_t *),
      NULL,
      NULL,
   };

   ckr->cur_scope = &ckr->scope;
   ckr->file_scope = &ckr->scope;
   ckr->error = false;

   checker_check_start(AST, ckr);
   checker_check_end(AST, ckr);

   node_walker(AST, _check_start, _check_end);

   return ckr->errors == 0;
}

void checker_check_start(node_t *node, checker_t *ckr) {
   static checker_t *check = NULL;
   if (ckr != NULL) check = ckr;
   else {
      switch (node->type) {
         case NODE_FILE: checker_check_file(check, node); return;
         case NODE_BLOCK: checker_check_block(check, node); return;
         case NODE_DEREF_EXPRESSION: checker_check_deref(check, node); return;
         case NODE_CALL_EXPRESSION: checker_check_callexpr(check, node); return;
         case NODE_IDENTIFIER: checker_check_identifier(check, node); return;
         case NODE_NUMBER_LITERAL:
         case NODE_STRING_LITERAL: checker_check_literal(check, node); return;
         case NODE_DATA_TYPE: checker_check_data_type(check, node); return;
         case NODE_VARIABLE_DECLARATION: checker_check_var_decl(check, node); return;
         case NODE_FUNCTION_DECLARATION: checker_check_func_decl(check, node); return;
         case NODE_RETURN: checker_check_return(check, node); return;
      }
   }

}

void checker_check_end(node_t *node, checker_t *ckr) {
   static checker_t *check = NULL;
   if (ckr != NULL) check = ckr;
   else {
      switch (node->type) {
         case NODE_BLOCK: checker_check_block_end(check, node); return;
         case NODE_FUNCTION_DECLARATION: checker_check_func_decl_end(check, node); return;
      }
   }
}

bool checker_check_file(checker_t *ckr, node_t *file) {
   node_def(file, FILE);

   for (int i = 0; i < dy_len(node->stmts); i++) {
      node_t *stmt = dyi(node->stmts)[i];

      switch (stmt->type) {
         case NODE_VARIABLE_DECLARATION:
            if (ht_exists_sv(ckr->cur_scope->decls, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value)) error("Redeclaration of Variable!");

            ht_set_sv(ckr->cur_scope->decls, stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value, checker_infer_var_decl(ckr, stmt));
            break;
         case NODE_EMPTY: continue;
         default: error("Statment Cant be File Scope!");
      }
   }

   return true;
}

bool checker_check_block(checker_t *ckr, node_t *block) {
   scope_t *scope = malloc(sizeof(scope_t));

   scope->decls = ht_init_sv(type_t *);
   scope->parent = ckr->cur_scope;
   scope->ret = ckr->cur_scope->ret;

   ckr->cur_scope = scope;

   return true;
}

bool checker_check_block_end(checker_t *ckr, node_t *block) {
   scope_t *scope = ckr->cur_scope;

   ht_free(scope->decls);
   ckr->cur_scope = scope->parent;

   free(scope);

   return true;
}

bool checker_check_binexpr(checker_t *ckr, node_t *expr);
bool checker_check_uryexpr(checker_t *ckr, node_t *expr);

bool checker_check_deref(checker_t *ckr, node_t *expr) {
   node_def(expr, DEREF_EXPRESSION);

   if (!type_is_ptr(checker_infer_expression(ckr, node->expr))) error("Cannot Derefrence Non Ptr Type!");

   return true;
}

bool checker_check_callexpr(checker_t *ckr, node_t *expr) {
   node_def(expr, CALL_EXPRESSION);

   type_t *type = checker_infer_callexpr_funct(ckr, expr);
   if (type == NULL) return false;

   if (dy_len(type->args) != dy_len(node->args)) 
      error("Invalid Amount of Arguments!");

   for (int i = 0; i < dy_len(node->args); i++) {
      type_t *argt = checker_infer_expression(ckr, dyi(node->args)[i]);
      type_t *typt = checker_reslove_type(ckr, dyi(type->args)[i].type);

      if (!type_cmp(typt, argt)) error("Invalid Argument Type!");
   }

   return true;
}

bool checker_check_identifier(checker_t *ckr, node_t *ident) {
   node_def(ident, IDENTIFIER);

   if (checker_get_type(ckr, node->value) == NULL) {
      print(node->value);
      error("Unknown Identifier!");
   }

   return true;
}

bool checker_check_literal(checker_t *ckr, node_t *lit) {
   return true;
}

bool checker_check_data_type(checker_t *ckr, node_t *dtype) {
   node_def(dtype, DATA_TYPE);

   type_t *type = checker_reslove_base_type(ckr, node->type);
   if (type == NULL) error("Unknown Type!");

   if (type->type == TYPE_FUNCTION)
      for (int i = 0; i < dy_len(type->args); i++)
         if (checker_reslove_base_type(ckr, dyi(type->args)[i].type) == NULL) error("Unknown Type!");

   return true;
}

bool checker_check_var_decl(checker_t *ckr, node_t *vard) {
   node_def(vard, VARIABLE_DECLARATION);

   char *name    = node->ident->IDENTIFIER.value;
   type_t *type  = checker_reslove_type(ckr, node->type->DATA_TYPE.type);
   if (type == NULL) return false;

   if (ckr->cur_scope != ckr->file_scope && ht_exists_sv(ckr->cur_scope->decls, name)) error("Redeclaration of Variable!");

   if (node->expr->type == NODE_NONE) goto NO_ERROR;
   else {
      type_t *infer = checker_infer_expression(ckr, node->expr);
      if (infer == NULL) return false;
      if (infer->type == TYPE_UNTYPED) infer = infer->uninfer;

      if (type->type == TYPE_INFER) {
         type = infer;
         node->type->DATA_TYPE.type = infer;

         goto NO_ERROR;
      }

      if (!type_cmp(type, infer)) error("Variable Declaration Types do Not Match!");
   }

NO_ERROR:
   ht_set_sv(ckr->cur_scope->decls, name, type);

   return true;
}

bool checker_check_func_decl(checker_t *ckr, node_t *funcd) {
   node_def(funcd, FUNCTION_DECLARATION);

   type_t *type = node->type->DATA_TYPE.type;

   scope_t *scope = malloc(sizeof(scope_t));

   scope->decls = ht_init_sv(type_t *);
   scope->parent = ckr->cur_scope;
   scope->ret = type->ret;

   ckr->cur_scope = scope;

   for (int i = 0; i < dy_len(type->args); i++) {
      if (ht_exists_sv(ckr->cur_scope->decls, dyi(type->args)[i].name)) error("Redefiation of Variable!");

      ht_set_sv(ckr->cur_scope->decls, dyi(type->args)[i].name, dyi(type->args)[i].type);
   }

   return true;
}

bool checker_check_func_decl_end(checker_t *ckr, node_t *funcd) {
   scope_t *scope = ckr->cur_scope;

   ht_free(scope->decls);
   ckr->cur_scope = scope->parent;

   free(scope);

   return true;
}

bool checker_check_if(checker_t *ckr, node_t *stmt);
bool checker_check_for(checker_t *ckr, node_t *stmt);
bool checker_check_break(checker_t *ckr, node_t *brk);
bool checker_check_continue(checker_t *ckr, node_t *cnt);

bool checker_check_return(checker_t *ckr, node_t *retn) {
   node_def(retn, RETURN);

   if (!type_cmp(ckr->cur_scope->ret, checker_infer_expression(ckr, node->value)))
      error("Invalid Return Type!");

   return true;
}
#include "checker.h"

bool checker_check(node_t *AST) {
   checker_t *ckr = malloc(sizeof(checker_t));

   ckr->file_decls = ht_init_sv(type_t *);
   ckr->scope_decls = ht_init_sv(type_t *);

   checker_file(ckr, AST);
}

void checker_file(checker_t *ckr, node_t *file) {
   for (int i = 0; i < dy_len(file->FILE.stmts); i++) {
      node_t *stmt = dyi(file->FILE.stmts)[i];

      if (stmt->type != NODE_VARIABLE_DECLARATION) continue;

      char *name = span_to_str(&stmt->VARIABLE_DECLARATION.ident->IDENTIFIER.value);

      // printf("%d\n", stmt->VARIABLE_DECLARATION.type->DATA_TYPE.type->type);
      if (stmt->VARIABLE_DECLARATION.type->DATA_TYPE.type->type == TYPE_INFER)
         ht_set_sv(ckr->file_decls, name, checker_infer_stmt_type(ckr, stmt->VARIABLE_DECLARATION.expr));
      else
         ht_set_sv(ckr->file_decls, name, stmt->VARIABLE_DECLARATION.type->DATA_TYPE.type);

      free(name);
   }

   print_span(&ht_get_sv(ckr->file_decls, "main")->ret->name);
   printf("%d\n", ht_get_sv(ckr->file_decls, "main")->type);
}

type_t *checker_infer_stmt_type(checker_t *ckr, node_t *stmt) {
   switch (stmt->type) {
      case NODE_FUNCTION_DECLARATION:
         return stmt->FUNCTION_DECLARATION.type->DATA_TYPE.type;
   }
}
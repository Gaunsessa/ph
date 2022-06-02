// #include "desugar.h"

// void _desugar_start(node_t *node) {
//    desugar_start(node, NULL);
// }

// void _desugar_end(node_t *node) {
//    desugar_end(node, NULL);
// }

// node_t *desugar_desugar(node_t *AST) {
//    desugar_t des = { 
//       NULL, 
//       0, 
//       node_init(NODE_SOURCE, .SOURCE = { dy_init(node_t *), dy_init(node_t *) }), 
//       dy_init(node_t *), 
//       NULL 
//    };

//    desugar_start(NULL, &des);
//    desugar_end(NULL, &des);

//    node_walker(AST, desugar_special, _desugar_start, _desugar_end);

//    dy_free(des.parent_scopes);

//    return des.AST;
// }

// bool desugar_special(node_t *node) {
//    return false;
// }

// void desugar_start(node_t *node, desugar_t *des) {
//    static desugar_t *desg = NULL;
//    if (des != NULL) desg = des;
//    else {
//       switch (node->type) {
//          case NODE_FILE: desugar_file(node, desg); break;
//          case NODE_VARIABLE_DECLARATION: desugar_var_decl(node, desg); break;
//          case NODE_IDENTIFIER: desugar_ident(node, desg); break;
//          case NODE_FEILD_EXPRESSION: desugar_feild_expr(node, desg); break;
//          case NODE_METHOD_EXPRESSION: desugar_method_expr(node, desg); break;
//          case NODE_DATA_TYPE: desugar_data_type(node, desg); break;
//          default: break;
//       }

//       switch (node->type) {
//          case NODE_FILE:
//          case NODE_FUNCTION_DECLARATION:
//          case NODE_IMPL:
//             dy_push(desg->parent_scopes, node);
//             desg->parent_scope = node;
//             break;
//          default: break;
//       }
//    }
// }

// void desugar_end(node_t *node, desugar_t *des) {
//    static desugar_t *desg = NULL;
//    if (des != NULL) desg = des;
//    else {
//       switch (node->type) {
//          default: break;
//       }

//       switch (node->type) {
//          case NODE_FILE:
//          case NODE_FUNCTION_DECLARATION:
//          case NODE_IMPL:
//             if (dy_len(desg->parent_scopes) > 0) {
//                dy_pop(desg->parent_scopes);
//                desg->parent_scope = dy_len(desg->parent_scopes) ? 
//                   dyi(desg->parent_scopes)[dy_len(desg->parent_scopes) - 1] : NULL;
//             } else desg->parent_scope = NULL;
//             break;
//          default: break;
//       }
//    }
// }

// void desugar_file(node_t *file, desugar_t *des) {
//    node_def(file, FILE);

//    des->module_name = node->name;
//    des->module_name_len = wcslen(node->name);
// }

// void desugar_var_decl(node_t *vard, desugar_t *des) {
//    node_def(vard, VARIABLE_DECLARATION);

//    if (M_COMPARE(des->parent_scope->type, NODE_FILE, NODE_IMPL)) {
//       dy_push(
//          node->type->DATA_TYPE.type->type == TYPE_FUNCTION ? 
//             des->AST->SOURCE.funcs : 
//             des->AST->SOURCE.vars, 
//          vard
//       );
//    }
// }

// void desugar_ident(node_t *ident, desugar_t *des) {
//    node_def(ident, IDENTIFIER);

//    if (M_COMPARE(des->parent_scope->type, NODE_FILE, NODE_IMPL)) {

//       node->value = desugar_rename_ident2(node->value, des);

//       // printf("%ls\n", node->value);
//    }
// }

// void desugar_feild_expr(node_t *expr, desugar_t *des) {
//    node_def(expr, FEILD_EXPRESSION);

//    // if (node->module) {
//    //    wchar_t *mod   = node->expr->IDENTIFIER.value;
//    //    wchar_t *ident = node->member;

//    //    size_t mod_len = wcslen(mod);

//    //    wchar_t *nident = malloc((mod_len + wcslen(ident) + 2) * sizeof(wchar_t));

//    //    wcscpy(nident, mod);
//    //    nident[mod_len] = L'_';

//    //    wcscpy(nident + mod_len + 1, ident);

//    //    node_free(expr);
//    //    memset(expr, 0, sizeof(node_t));

//    //    expr->type = NODE_IDENTIFIER;
//    //    expr->IDENTIFIER.value = nident;
//    // }
// }

// void desugar_method_expr(node_t *expr, desugar_t *des) {
//    node_def(expr, METHOD_EXPRESSION);

//    // if (node->module) {
//    //    wchar_t *mod   = node->expr->IDENTIFIER.value;
//    //    wchar_t *ident = node->member;

//    //    size_t mod_len = wcslen(mod);

//    //    wchar_t *nident = malloc((mod_len + wcslen(ident) + 2) * sizeof(wchar_t));

//    //    wcscpy(nident, mod);
//    //    nident[mod_len] = L'_';

//    //    wcscpy(nident + mod_len + 1, ident);

//    //    node_t nc = *expr;
//    //    memset(expr, 0, sizeof(node_t));

//    //    expr->type = NODE_CALL_EXPRESSION;
//    //    expr->CALL_EXPRESSION.args = nc.METHOD_EXPRESSION.args;
//    //    expr->CALL_EXPRESSION.curried = nc.METHOD_EXPRESSION.curried;
//    //    expr->CALL_EXPRESSION.func = nc.METHOD_EXPRESSION.expr;

//    //    free(expr->CALL_EXPRESSION.func->IDENTIFIER.value);
//    //    // free(node->member);

//    //    expr->CALL_EXPRESSION.func->IDENTIFIER.value = nident;
//    // }
// }

// void desugar_data_type(node_t *dtype, desugar_t *des) {
//    node_def(dtype, DATA_TYPE);

//    desugar_type(node->type, des);
// }

// void desugar_type(type_t *type, desugar_t *des) {
//    type = desugar_resolve_type(type);

//    // if (type->type == TYPE_ALIAS) {
//    //    type->name = desugar_rename_ident2(type->name, des);
//    // } else if (type->type == TYPE_FUNCTION) {
//    //    for (int i = 0; i < dy_len(type->args); i++)
//    //       desugar_type(dyi(type->args)[i].type, des);

//    //    desugar_type(type->ret, des);
//    // }
// }

// wchar_t *desugar_rename_ident2(wchar_t *ident, desugar_t *des) {
//    dynarr_t(wchar_t *) prefix = dy_init(wchar_t *);
//    size_t prefix_len          = wcslen(ident) + 1;

//    for (int i = 0; i < dy_len(des->parent_scopes); i++) {
//       node_t *scope = dyi(des->parent_scopes)[i];

//       if (scope->type == NODE_FILE) {
//          dy_push(prefix, scope->FILE.name);

//          prefix_len += wcslen(dyi(prefix)[dy_len(prefix) - 1]) + 1;
//       } else if (scope->type == NODE_IMPL) {
//          dy_push(prefix, L"impl");
//          dy_push(prefix, scope->IMPL.type->DATA_TYPE.type->name);

//          prefix_len += wcslen(dyi(prefix)[dy_len(prefix) - 1]) + 4 + 2;
//       }
//    }

//    wchar_t *nident = malloc(prefix_len * sizeof(wchar_t));

//    size_t pos = 0;
//    for (int i = 0; i < dy_len(prefix); i++) {
//       wcscpy(nident + pos, dyi(prefix)[i]);
//       pos += wcslen(dyi(prefix)[i]) + 1;
//       nident[pos - 1] = L'_';
//    }

//    wcscpy(nident + pos, ident);

//    free(ident);

//    return nident;
// }

// // TODO: so many duplicate wcslens
// wchar_t *desugar_rename_ident(desugar_t *des, wchar_t *ident, size_t amt, wchar_t **args) {
//    size_t ident_len = wcslen(ident);
//    size_t len       = des->module_name_len + ident_len + 2;

//    for (int i = 0; i < amt; i++)
//       len += wcslen(args[i]) + 1;

//    ident = realloc(ident, len * sizeof(wchar_t));
//    memmove(ident + len - ident_len - 1, ident, (ident_len + 1) * sizeof(wchar_t));

//    wcscpy(ident, des->module_name);
//    ident[des->module_name_len] = L'_';

//    int pos = des->module_name_len + 1;
//    for (int i = 0; i < amt; i++) {
//       wchar_t *str = args[i];
//       size_t len   = wcslen(str);

//       memcpy(ident + pos, str, len * sizeof(wchar_t));

//       pos += len + 1;
//       ident[pos - 1] = L'_';
//    }

//    return ident;
// }

// type_t *desugar_resolve_type(type_t *type) {
//    if (type == NULL) return NULL;

//    switch (type->type) {
//       case TYPE_PTR: return desugar_resolve_type(type->ptr_base);
//       case TYPE_ARRAY: return desugar_resolve_type(type->arr_base);
//       case TYPE_UNTYPED: return desugar_resolve_type(type->uninfer);

//       default: return type;
//    }
// }
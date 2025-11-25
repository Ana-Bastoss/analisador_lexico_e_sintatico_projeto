#ifndef AST_H
#define AST_H

#include "tokens.h"

typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VAR_DECLS,
    AST_VARDECL,
    AST_TYPE,
    AST_ID,
    AST_INT_LIT,
    AST_REAL_LIT,
    AST_COMPOUND,
    AST_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_BINOP,
    AST_UNOP,
    AST_RELOP,
    AST_BINOMIAL
} NodeKind;

typedef struct AST {
    NodeKind kind;
    char lexeme[256];
    struct AST *child;
    struct AST *sibling;
} AST;

AST *ast_new(NodeKind kind, const char *lexeme);
void ast_add_child(AST *parent, AST *child);
void ast_add_sibling(AST *node, AST *sibling);
void ast_print(AST *root, int level);
void ast_free(AST *root);
// Função para gerar o arquivo DOT
void ast_to_dot(AST *root, const char *filename);


#endif

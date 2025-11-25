#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

AST *ast_new(NodeKind kind, const char *lexeme) {
    AST *n = (AST *)malloc(sizeof(AST));
    if (!n) { perror("falha ao alocar AST"); exit(1); }
    n->kind = kind;
    n->child = NULL;
    n->sibling = NULL;
    if (lexeme) strncpy(n->lexeme, lexeme, sizeof(n->lexeme));
    else n->lexeme[0] = '\0';
    n->lexeme[sizeof(n->lexeme) - 1] = '\0';
    return n;
}

void ast_add_child(AST *parent, AST *child) {
    if (!parent || !child) return;
    if (!parent->child) parent->child = child;
    else {
        AST *p = parent->child;
        while (p->sibling) p = p->sibling;
        p->sibling = child;
    }
}

void ast_add_sibling(AST *node, AST *sibling) {
    if (!node || !sibling) return;
    while (node->sibling) node = node->sibling;
    node->sibling = sibling;
}

static const char* kind_to_string(NodeKind k) {
    switch (k) {
        case AST_PROGRAM:   return "PROGRAM";
        case AST_BLOCK:     return "BLOCK";
        case AST_VAR_DECLS: return "VAR_DECLS";
        case AST_VARDECL:   return "VARDECL";
        case AST_TYPE:      return "TYPE";
        case AST_ID:        return "ID";
        case AST_INT_LIT:   return "INT_LIT";
        case AST_REAL_LIT:  return "REAL_LIT";
        case AST_COMPOUND:  return "COMPOUND";
        case AST_ASSIGN:    return "ASSIGN";
        case AST_IF:        return "IF";
        case AST_WHILE:     return "WHILE";
        case AST_BINOP:     return "BINOP";
        case AST_UNOP:      return "UNOP";
        case AST_RELOP:     return "RELOP";
        case AST_BINOMIAL:  return "BINOMIAL"; // <--- ADICIONADO
        default:            return "UNKNOWN";
    }
}

static void print_indent(int level) {
    for (int i = 0; i < level; i++) printf("  ");
}

void ast_print(AST *root, int level) {
    if (!root) return;
    print_indent(level);
    printf("%s", kind_to_string(root->kind));
    if (root->lexeme[0] != '\0') printf(" (%s)", root->lexeme);
    printf("\n");
    ast_print(root->child, level + 1);
    ast_print(root->sibling, level);
}

void ast_free(AST *root) {
    if (!root) return;
    ast_free(root->child);
    ast_free(root->sibling);
    free(root);
}

// --- GERAÇÃO DE ARQUIVO DOT (Para o Gráfico) ---
static void ast_print_dot_rec(AST *node, FILE *f, int *next_id) {
    if (!node) return;
    int my_id = *next_id;
    
    const char *str_kind = kind_to_string(node->kind);
    if (strlen(node->lexeme) > 0) {
        fprintf(f, "  n%d [label=\"%s\\n(%s)\"];\n", my_id, str_kind, node->lexeme);
    } else {
        fprintf(f, "  n%d [label=\"%s\"];\n", my_id, str_kind);
    }

    AST *child = node->child;
    while (child) {
        (*next_id)++;
        int child_id = *next_id;
        fprintf(f, "  n%d -> n%d;\n", my_id, child_id);
        ast_print_dot_rec(child, f, next_id);
        child = child->sibling;
    }
}

void ast_to_dot(AST *root, const char *filename) {
    if (!root) return;
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Erro ao criar arquivo .dot"); return; }
    fprintf(f, "digraph AST {\n");
    fprintf(f, "  node [shape=box, fontname=\"Courier\", style=filled, fillcolor=white];\n");
    int id = 0;
    ast_print_dot_rec(root, f, &id);
    fprintf(f, "}\n");
    fclose(f);
}
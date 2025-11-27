#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#ifdef _WIN32
  #include <windows.h>
#endif

#include "tokens.h"
#include "ast.h"

static void configurar_utf8(void) {
    if (!setlocale(LC_ALL, "C.UTF-8"))
        if (!setlocale(LC_ALL, "pt_BR.UTF-8"))
            setlocale(LC_ALL, "");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

typedef struct {
    Token current;
    int print_rules;
} Parser;

static AST* programa(Parser *ps);
static AST* bloco(Parser *ps);
static AST* parte_decl_var(Parser *ps);
static AST* declaracao_var(Parser *ps);
static AST* lista_identificadores(Parser *ps);
static AST* tipo(Parser *ps);
static AST* comando_composto(Parser *ps);
static AST* comando(Parser *ps);
static AST* atribuicao(Parser *ps);
static AST* comando_condicional(Parser *ps);
static AST* comando_repetitivo(Parser *ps);
static AST* expressao(Parser *ps);
static AST* expressao_simples(Parser *ps);
static AST* termo(Parser *ps);
static AST* fator(Parser *ps);

static void print_rule(Parser *ps, const char *rule) {
    if (ps->print_rules) puts(rule);
}

static void error_unexpected(Token tk) {
    fprintf(stderr, "%d:token nao esperado [%s].\n", tk.linha, tk.lexema);
    exit(1);
}

static void error_unexpected_eof(int line) {
    fprintf(stderr, "%d:fim de arquivo não esperado.\n", line);
    exit(1);
}

static Token CasaToken(Parser *ps, TipoToken esperado) {
    Token t = ps->current;
    if (ps->current.tipo == esperado) {
        ps->current = obterProximoToken();
        if (ps->current.tipo == TOKEN_ERRO) error_unexpected(ps->current);
        return t;
    } else {
        if (ps->current.tipo == TOKEN_FIM_DE_ARQUIVO)
            error_unexpected_eof(ps->current.linha);
        else
            error_unexpected(ps->current);
    }
    return t;
}

static AST* programa(Parser *ps) {
    print_rule(ps, "<programa> ::= program <identificador> ; <bloco> .");
    CasaToken(ps, TOKEN_KEY_PROGRAM);
    Token id = CasaToken(ps, TOKEN_ID);
    CasaToken(ps, TOKEN_SMB_SEM);
    
    AST *node_prog = ast_new(AST_PROGRAM, "program");
    AST *node_id   = ast_new(AST_ID, id.lexema);
    ast_add_child(node_prog, node_id);
    
    AST *node_block = bloco(ps);
    ast_add_child(node_id, node_block);
    
    CasaToken(ps, TOKEN_SMB_DOT);
    return node_prog;
}

static AST* bloco(Parser *ps) {
    print_rule(ps, "<bloco> ::= <parte de declaracoes> <comando composto>");
    AST *node_block = ast_new(AST_BLOCK, "block");
    if (ps->current.tipo == TOKEN_KEY_VAR) {
        ast_add_child(node_block, parte_decl_var(ps));
    }
    ast_add_child(node_block, comando_composto(ps));
    return node_block;
}

static AST* parte_decl_var(Parser *ps) {
    print_rule(ps, "<parte de declaracoes> ::= var ...");
    AST *var_decls = ast_new(AST_VAR_DECLS, "var_decls");
    CasaToken(ps, TOKEN_KEY_VAR);
    
    do {
        AST *decl = declaracao_var(ps);
        ast_add_child(var_decls, decl);
        CasaToken(ps, TOKEN_SMB_SEM);
    } while (ps->current.tipo == TOKEN_ID);
    
    return var_decls;
}

static AST* declaracao_var(Parser *ps) {
    AST *node = ast_new(AST_VARDECL, "var");
    AST *ids = lista_identificadores(ps);
    ast_add_child(node, ids);
    CasaToken(ps, TOKEN_SMB_COLON);
    AST *tp = tipo(ps);
    ast_add_child(node, tp);
    return node;
}

static AST* lista_identificadores(Parser *ps) {
    Token id = CasaToken(ps, TOKEN_ID);
    AST *first = ast_new(AST_ID, id.lexema);
    AST *last = first;
    while (ps->current.tipo == TOKEN_SMB_COM) {
        CasaToken(ps, TOKEN_SMB_COM);
        Token next = CasaToken(ps, TOKEN_ID);
        AST *n = ast_new(AST_ID, next.lexema);
        ast_add_sibling(last, n);
        last = n;
    }
    return first;
}

static AST* tipo(Parser *ps) {
    if (ps->current.tipo == TOKEN_KEY_INTEGER || ps->current.tipo == TOKEN_KEY_REAL) {
        Token t = ps->current;
        ps->current = obterProximoToken();
        return ast_new(AST_TYPE, t.lexema);
    }
    error_unexpected(ps->current);
    return NULL;
}

static AST* comando_composto(Parser *ps) {
    print_rule(ps, "<comando composto> ::= begin <comandos> end");
    CasaToken(ps, TOKEN_KEY_BEGIN);
    AST *node_comp = ast_new(AST_COMPOUND, "begin_end");
    
    if (ps->current.tipo != TOKEN_KEY_END) {
        AST *cmd = comando(ps);
        if (cmd) ast_add_child(node_comp, cmd);
    }
    
    while (ps->current.tipo == TOKEN_SMB_SEM) {
        CasaToken(ps, TOKEN_SMB_SEM);
        if (ps->current.tipo == TOKEN_KEY_END) break;
        AST *next = comando(ps);
        if (next) ast_add_child(node_comp, next);
    }
    
    CasaToken(ps, TOKEN_KEY_END);
    return node_comp;
}

static AST* comando(Parser *ps) {
    switch (ps->current.tipo) {
        case TOKEN_ID: return atribuicao(ps);
        case TOKEN_KEY_BEGIN: return comando_composto(ps);
        case TOKEN_KEY_IF: return comando_condicional(ps);
        case TOKEN_KEY_WHILE: return comando_repetitivo(ps);
        default: return NULL; // Comando vazio ou erro
    }
}

static AST* atribuicao(Parser *ps) {
    Token id = CasaToken(ps, TOKEN_ID);
    CasaToken(ps, TOKEN_OP_ASS);
    AST *node_assign = ast_new(AST_ASSIGN, ":=");
    AST *node_var = ast_new(AST_ID, id.lexema);
    ast_add_child(node_assign, node_var);
    ast_add_child(node_assign, expressao(ps));
    return node_assign;
}

static AST* comando_condicional(Parser *ps) {
    CasaToken(ps, TOKEN_KEY_IF);
    AST *node_if = ast_new(AST_IF, "if");
    ast_add_child(node_if, expressao(ps));
    CasaToken(ps, TOKEN_KEY_THEN);
    ast_add_child(node_if, comando(ps));
    if (ps->current.tipo == TOKEN_KEY_ELSE) {
        CasaToken(ps, TOKEN_KEY_ELSE);
        ast_add_child(node_if, comando(ps));
    }
    return node_if;
}

static AST* comando_repetitivo(Parser *ps) {
    CasaToken(ps, TOKEN_KEY_WHILE);
    AST *node_while = ast_new(AST_WHILE, "while");
    ast_add_child(node_while, expressao(ps));
    CasaToken(ps, TOKEN_KEY_DO);
    ast_add_child(node_while, comando(ps));
    return node_while;
}

static AST* expressao(Parser *ps) {
    print_rule(ps, "<expressao> ::= <expressao simples> [ <relacao> <expressao simples> ]");

    AST *left = expressao_simples(ps);
  
    if (ps->current.tipo == TOKEN_OP_EQ || ps->current.tipo == TOKEN_OP_NE ||
        ps->current.tipo == TOKEN_OP_LT || ps->current.tipo == TOKEN_OP_LE ||
        ps->current.tipo == TOKEN_OP_GE || ps->current.tipo == TOKEN_OP_GT) {

        Token op = ps->current;
        ps->current = obterProximoToken();
        if (ps->current.tipo == TOKEN_ERRO) error_unexpected(ps->current);

        AST *node_rel = ast_new(AST_RELOP, op.lexema);
        ast_add_child(node_rel, left);

        AST *right = expressao_simples(ps);
        ast_add_child(node_rel, right);

        return node_rel;
    }

    return left;
}

static AST* expressao_simples(Parser *ps) {
    if (ps->current.tipo == TOKEN_OP_AD || ps->current.tipo == TOKEN_OP_MIN) {
        Token op = ps->current;
        ps->current = obterProximoToken();
        AST *un = ast_new(AST_UNOP, op.lexema);
        ast_add_child(un, termo(ps));
        return un; // Simplificação: unário seguido de nada
    }
    AST *node = termo(ps);
    while (ps->current.tipo == TOKEN_OP_AD || ps->current.tipo == TOKEN_OP_MIN) {
        Token op = ps->current;
        ps->current = obterProximoToken();
        AST *bin = ast_new(AST_BINOP, op.lexema);
        ast_add_child(bin, node);
        ast_add_child(bin, termo(ps));
        node = bin;
    }
    return node;
}

static AST* termo(Parser *ps) {
    AST *node = fator(ps);
    while (ps->current.tipo == TOKEN_OP_MUL || ps->current.tipo == TOKEN_OP_DIV) {
        Token op = ps->current;
        ps->current = obterProximoToken();
        AST *bin = ast_new(AST_BINOP, op.lexema);
        ast_add_child(bin, node);
        ast_add_child(bin, fator(ps));
        node = bin;
    }
    return node;
}

static AST* fator(Parser *ps) {
    if (ps->current.tipo == TOKEN_ID) {
        return ast_new(AST_ID, CasaToken(ps, TOKEN_ID).lexema);
    } else if (ps->current.tipo == TOKEN_LIT_INT) {
        return ast_new(AST_INT_LIT, CasaToken(ps, TOKEN_LIT_INT).lexema);
    } else if (ps->current.tipo == TOKEN_LIT_REAL) {
        return ast_new(AST_REAL_LIT, CasaToken(ps, TOKEN_LIT_REAL).lexema);
    } else if (ps->current.tipo == TOKEN_SMB_OPA) {
        CasaToken(ps, TOKEN_SMB_OPA);
        AST *node = expressao(ps);
        CasaToken(ps, TOKEN_SMB_CPA);
        return node;
    } else if (ps->current.tipo == TOKEN_EXP_BINOMIAL) {
        Token t = CasaToken(ps, TOKEN_EXP_BINOMIAL);
        return ast_new(AST_BINOMIAL, t.lexema); // Nó específico para o futuro!
    } else {
        error_unexpected(ps->current);
        return NULL;
    }
}

int main(int argc, char **argv) {
    configurar_utf8();
    if (argc < 2) {
        fprintf(stderr, "uso: %s fonte.pas\n", argv[0]);
        return 1;
    }
    arquivo_fonte = fopen(argv[1], "r");
    if (!arquivo_fonte) { perror("abrindo fonte"); return 1; }

    Parser ps;
    ps.print_rules = 0;
    ps.current = obterProximoToken();
    if (ps.current.tipo == TOKEN_ERRO) error_unexpected(ps.current);

    AST *root = programa(&ps);

    puts("Árvore sintática (Texto):");
    ast_print(root, 0);
    
    puts("\nGerando arquivos visuais...");
    ast_to_dot(root, "arvore.dot");
    int ret = system("dot -Tpng arvore.dot -o arvore.png");
    if(ret == 0) puts("Sucesso! 'arvore.png' gerado.");
    else puts("Aviso: Graphviz (dot) não encontrado.");

    ast_free(root);
    fclose(arquivo_fonte);
    return 0;
}

/*
  ANALISADOR LÉXICO (scanner)
  - Versão Final Corrigida
  - Integração com Parser: Auto-inicialização da tabela.
  - Correção: Remoção da inicialização duplicada no main de teste.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <locale.h>
#ifdef _WIN32
  #include <windows.h>
  #define strcasecmp _stricmp
#endif
#include "tokens.h"

/* ==================== Tabela de símbolos ==================== */

typedef struct {
    char lexema[100];
    TipoToken tipo;
} EntradaTabelaSimbolos;

#define TAMANHO_TABELA_SIMBOLOS 200
static EntradaTabelaSimbolos tabela_de_simbolos[TAMANHO_TABELA_SIMBOLOS];
static int contador_tabela_simbolos = 0;

/* CORREÇÃO VISUAL: Nomes bonitinhos com prefixo para o arquivo .lex */
const char* tipo_token_para_string[] = {
    "KEY_PROGRAM", "KEY_VAR", "KEY_INTEGER", "KEY_REAL", "KEY_BEGIN", "KEY_END",
    "KEY_IF", "KEY_THEN", "KEY_ELSE", "KEY_WHILE", "KEY_DO",
    "ID",
    "LIT_INT", "LIT_REAL", "LIT_STRING",
    "OP_EQ", "OP_GE", "OP_MUL", "OP_NE", "OP_LE",
    "OP_DIV", "OP_GT", "OP_AD", "OP_LT", "OP_MIN", "OP_POW",
    "OP_ASS",
    "SMB_COM", "SMB_SEM", "SMB_OPA", "SMB_CPA", "SMB_COLON",
    "SMB_DOT",
    "EXP_BINOMIAL", 
    "FIM_DE_ARQUIVO", "ERRO"
};

static void inicializarTabelaDeSimbolos(void) {
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "program");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_PROGRAM;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "var");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_VAR;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "integer");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_INTEGER;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "real");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_REAL;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "begin");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_BEGIN;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "end");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_END;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "if");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_IF;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "then");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_THEN;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "else");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_ELSE;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "while");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_WHILE;
    strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, "do");
    tabela_de_simbolos[contador_tabela_simbolos++].tipo = TOKEN_KEY_DO;
}

static TipoToken consultarOuInserirSimbolo(const char* lexema) {
    for (int i = 0; i < contador_tabela_simbolos; i++) {
        if (strcasecmp(tabela_de_simbolos[i].lexema, lexema) == 0) {
            return tabela_de_simbolos[i].tipo;
        }
    }
    if (contador_tabela_simbolos < TAMANHO_TABELA_SIMBOLOS) {
        strcpy(tabela_de_simbolos[contador_tabela_simbolos].lexema, lexema);
        tabela_de_simbolos[contador_tabela_simbolos].tipo = TOKEN_ID;
        contador_tabela_simbolos++;
        return TOKEN_ID;
    }
    fprintf(stderr, "ERRO FATAL: Tabela de Símbolos cheia.\n");
    return TOKEN_ERRO;
}

#ifdef LEX_MAIN
static void imprimirTabelaDeSimbolos(void) {
    printf("\n--- Tabela de Símbolos Final ---\n");
    for (int i = 0; i < contador_tabela_simbolos; i++) {
        const char *nome = "UNKNOWN";
        if (tabela_de_simbolos[i].tipo >= 0) {
             nome = tipo_token_para_string[tabela_de_simbolos[i].tipo];
        }
        printf("  [%03d] Lexema: %-20s | Token: %s\n",
               i, tabela_de_simbolos[i].lexema, nome);
    }
    printf("----------------------------------\n");
}
#endif

FILE* arquivo_fonte = NULL;    
static int linha_atual  = 1;
static int coluna_atual = 1;


static int proximoCaractere(void) {
    int c = fgetc(arquivo_fonte);
    if (c == '\n') {
        linha_atual++;
        coluna_atual = 1;
    } else if (c != EOF) {
        coluna_atual++;
    }
    return c;
}

static int preverCaractere(void) {
    int c = fgetc(arquivo_fonte);
    if (c != EOF) ungetc(c, arquivo_fonte); 
    return c;
}

static Token criarToken(TipoToken tipo, const char* lexema, int linha, int coluna) {
    Token t; t.tipo = tipo;
    strcpy(t.lexema, lexema);
    t.linha = linha; t.coluna = coluna;
    return t;
}

static void pularEspacos(void) {
    int p;
    while ((p = preverCaractere()) != EOF &&
           (isspace(p) || (unsigned)p == 160 || (unsigned)p == 194 || (unsigned)p == 195)) {
        proximoCaractere();
    }
}

/* ==================== Scanner Principal ==================== */

Token obterProximoToken(void) {
    // Auto-inicialização para funcionar com o Parser
    // Garante que a tabela seja carregada apenas uma vez,
    // não importa se chamado pelo parser ou pelo main de teste.
    static bool tabela_inicializada = false;
    if (!tabela_inicializada) {
        inicializarTabelaDeSimbolos();
        tabela_inicializada = true;
    }

    char lexema[256];
    int idx = 0;
    int c;

    for (;;) {
        c = proximoCaractere();
        int lin0 = linha_atual;
        int col0 = coluna_atual - 1;

        if (c == EOF) {
            return criarToken(TOKEN_FIM_DE_ARQUIVO, "EOF", linha_atual, coluna_atual);
        }

        if (isspace(c) || (unsigned)c == 160 || (unsigned)c == 194 || (unsigned)c == 195) {
            continue;
        }

        if (c == '{') {
            int linC = lin0, colC = col0;
            do {
                c = proximoCaractere();
                if (c == EOF) {
                    fprintf(stderr, "ERRO LEXICO: Comentario nao fechado (Linha %d)\n", linC);
                    return criarToken(TOKEN_ERRO, "{...", linC, colC);
                }
            } while (c != '}');
            continue;
        }

        if (isalpha(c) || c == '_') {
            lexema[idx++] = (char)c;
            int p;
            while ((p = preverCaractere()) != EOF && (isalnum(p) || p == '_')) {
                lexema[idx++] = (char)proximoCaractere();
            }
            lexema[idx] = '\0';
            TipoToken tipo = consultarOuInserirSimbolo(lexema);
            return criarToken(tipo, lexema, lin0, col0);
        }

        if (isdigit(c)) {
            lexema[idx++] = (char)c;
            int p;
            bool eh_real = false;
            while ((p = preverCaractere()) != EOF && isdigit(p)) {
                lexema[idx++] = (char)proximoCaractere();
            }
            if (preverCaractere() == '.') {
                int ponto = proximoCaractere();
                if (!isdigit(preverCaractere())) {
                    ungetc(ponto, arquivo_fonte);
                    lexema[idx] = '\0';
                    return criarToken(TOKEN_LIT_INT, lexema, lin0, col0);
                }
                eh_real = true;
                lexema[idx++] = (char)ponto;
                while ((p = preverCaractere()) != EOF && isdigit(p)) {
                    lexema[idx++] = (char)proximoCaractere();
                }
            }
            lexema[idx] = '\0';
            return criarToken(eh_real ? TOKEN_LIT_REAL : TOKEN_LIT_INT, lexema, lin0, col0);
        }

        if (c == '\'') {
            idx = 0;
            while ((c = proximoCaractere()) != '\'' && c != '\n' && c != EOF) {
                if (idx < 255) lexema[idx++] = (char)c;
            }
            lexema[idx] = '\0';
            if (c == '\'') return criarToken(TOKEN_LIT_STRING, lexema, lin0, col0);
            else {
                fprintf(stderr, "ERRO LEXICO: String nao fechada (Linha %d)\n", lin0);
                return criarToken(TOKEN_ERRO, lexema, lin0, col0);
            }
        }

        if (c == '<') {
            int p = preverCaractere();
            if (p == '>') { proximoCaractere(); return criarToken(TOKEN_OP_NE, "<>", lin0, col0); }
            if (p == '=') { proximoCaractere(); return criarToken(TOKEN_OP_LE, "<=", lin0, col0); }
            return criarToken(TOKEN_OP_LT, "<", lin0, col0);
        }
        if (c == '>') {
            if (preverCaractere() == '=') { proximoCaractere(); return criarToken(TOKEN_OP_GE, ">=", lin0, col0); }
            return criarToken(TOKEN_OP_GT, ">", lin0, col0);
        }
        if (c == '=') return criarToken(TOKEN_OP_EQ, "=", lin0, col0);

        switch (c) {
            case '+': return criarToken(TOKEN_OP_AD, "+", lin0, col0);
            case '-': return criarToken(TOKEN_OP_MIN, "-", lin0, col0);
            case '*': return criarToken(TOKEN_OP_MUL, "*", lin0, col0);
            case '/': return criarToken(TOKEN_OP_DIV, "/", lin0, col0);
            case ',': return criarToken(TOKEN_SMB_COM, ",", lin0, col0);
            case ';': return criarToken(TOKEN_SMB_SEM, ";", lin0, col0);
            case '(': {
                // --- LÓGICA DE BINÔMIO ---
                long pos_inicial = ftell(arquivo_fonte);
                int linha_bkp = linha_atual;
                int coluna_bkp = coluna_atual;
                
                char buffer_lexema[256];
                int idx_buf = 0;
                buffer_lexema[idx_buf++] = '(';

                pularEspacos();
                int p = preverCaractere();
                
                // 1. Primeiro Termo (Letra, Numero ou Underscore)
                if (!isalnum(p) && p != '_') goto falha_binomial;
                while (isalnum(p) || p == '.' || p == '_') {
                    buffer_lexema[idx_buf++] = (char)proximoCaractere();
                    p = preverCaractere();
                }

                pularEspacos();
                // 2. Operador
                p = preverCaractere();
                if (p != '+' && p != '-') goto falha_binomial;
                buffer_lexema[idx_buf++] = (char)proximoCaractere();

                pularEspacos();
                // 3. Segundo Termo
                p = preverCaractere();
                if (!isalnum(p) && p != '.' && p != '_') goto falha_binomial;
                while (isalnum(p) || p == '.' || p == '_') {
                    buffer_lexema[idx_buf++] = (char)proximoCaractere();
                    p = preverCaractere();
                }

                pularEspacos();
                // 4. Fecha Parentese
                if (preverCaractere() != ')') goto falha_binomial;
                buffer_lexema[idx_buf++] = (char)proximoCaractere();

                pularEspacos();
                // 5. Chapeu e Expoente
                if (preverCaractere() != '^') goto falha_binomial;
                buffer_lexema[idx_buf++] = (char)proximoCaractere();

                pularEspacos();
                if (!isdigit(preverCaractere())) goto falha_binomial;
                while (isdigit(preverCaractere())) {
                    buffer_lexema[idx_buf++] = (char)proximoCaractere();
                }

                buffer_lexema[idx_buf] = '\0';
                return criarToken(TOKEN_EXP_BINOMIAL, buffer_lexema, lin0, col0);

            falha_binomial:
                fseek(arquivo_fonte, pos_inicial, SEEK_SET);
                linha_atual = linha_bkp;
                coluna_atual = coluna_bkp;
                return criarToken(TOKEN_SMB_OPA, "(", lin0, col0);
            }
            case ')': return criarToken(TOKEN_SMB_CPA, ")", lin0, col0);
            case '^': return criarToken(TOKEN_OP_POW, "^", lin0, col0);
            case '.': return criarToken(TOKEN_SMB_DOT, ".", lin0, col0);
            case ':':
                if (preverCaractere() == '=') { proximoCaractere(); return criarToken(TOKEN_OP_ASS, ":=", lin0, col0); }
                else return criarToken(TOKEN_SMB_COLON, ":", lin0, col0);
            default:
                if (c != EOF && isprint((unsigned char)c)) {
                    char ch[2] = {(char)c, '\0'};
                    fprintf(stderr, "ERRO LÉXICO: Caractere desconhecido '%c' na linha %d, coluna %d.\n",
                            (char)c, lin0, col0);
                    return criarToken(TOKEN_ERRO, ch, lin0, col0);
                } else {
                    sprintf(lexema, "\\x%02X", (unsigned char)c);
                    fprintf(stderr, "ERRO LÉXICO: Caractere não imprimível (código %d) na linha %d, coluna %d.\n",
                            (unsigned char)c, lin0, col0);
                    return criarToken(TOKEN_ERRO, lexema, lin0, col0);
                }
        }
    }
}

/* ==================== Main OPCIONAL para gerar .lex ==================== */
#ifdef LEX_MAIN

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_fonte.pas>\n", argv[0]);
        return 1;
    }

    arquivo_fonte = fopen(argv[1], "r");
    if (!arquivo_fonte) { perror("Erro ao abrir arquivo fonte"); return 1; }

    char nome_lex[256];
    strncpy(nome_lex, argv[1], sizeof(nome_lex));
    nome_lex[sizeof(nome_lex)-1] = '\0';
    char *p = strrchr(nome_lex, '.');
    if (p && p != nome_lex) strcpy(p, ".lex"); else strncat(nome_lex, ".lex", sizeof(nome_lex)-strlen(nome_lex)-1);

    FILE* out = fopen(nome_lex, "w");
    if (!out) { perror("Erro ao criar arquivo de saida"); fclose(arquivo_fonte); return 1; }

    fprintf(out, "%-20s | %-17s | %-5s | %s\n", "LEXEMA", "TIPO DE TOKEN", "LINHA", "COLUNA");
    fprintf(out, "------------------------------------------------------------------\n");

    bool contem_erros = false;
    printf("Iniciando analise lexica...\n");

    for (;;) {
        Token tk = obterProximoToken();
        if (tk.tipo == TOKEN_ERRO) contem_erros = true;
        if (tk.tipo == TOKEN_FIM_DE_ARQUIVO) break;
        fprintf(out, "%-20s | %-17s | %-5d | %d\n",
                tk.lexema, tipo_token_para_string[tk.tipo], tk.linha, tk.coluna);
    }

    fclose(arquivo_fonte);
    fclose(out);
    imprimirTabelaDeSimbolos();
    return 0;
}
#endif

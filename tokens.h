#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>

typedef enum {
    // Palavras-chave
    TOKEN_KEY_PROGRAM, TOKEN_KEY_VAR, TOKEN_KEY_INTEGER, TOKEN_KEY_REAL, 
    TOKEN_KEY_BEGIN, TOKEN_KEY_END, TOKEN_KEY_IF, TOKEN_KEY_THEN, 
    TOKEN_KEY_ELSE, TOKEN_KEY_WHILE, TOKEN_KEY_DO,
    
    // Identificadores e Literais
    TOKEN_ID,
    TOKEN_LIT_INT, TOKEN_LIT_REAL, TOKEN_LIT_STRING,
    
    // Operadores
    TOKEN_OP_EQ, TOKEN_OP_GE, TOKEN_OP_MUL, TOKEN_OP_NE, TOKEN_OP_LE,
    TOKEN_OP_DIV, TOKEN_OP_GT, 
    TOKEN_OP_AD, // + (Atenção: Um D só, conforme seu padrão anterior)
    TOKEN_OP_LT, TOKEN_OP_MIN, TOKEN_OP_POW,
    TOKEN_OP_ASS, // :=
    
    // Símbolos
    TOKEN_SMB_COM, TOKEN_SMB_SEM, TOKEN_SMB_OPA, TOKEN_SMB_CPA, TOKEN_SMB_COLON,
    TOKEN_SMB_DOT,
    
    // Especial
    TOKEN_EXP_BINOMIAL, // <--- ADICIONADO AQUI
    
    // Controle
    TOKEN_FIM_DE_ARQUIVO, TOKEN_ERRO
} TipoToken;

typedef struct {
    TipoToken tipo;
    char lexema[256];
    int linha;
    int coluna;
} Token;

/* Tabela com o “nome bonitinho” de cada token (definida no léxico). */
extern const char* tipo_token_para_string[];

/* Coisas do léxico que o parser usa. */
extern FILE* arquivo_fonte;
Token obterProximoToken(void);

#endif /* TOKENS_H */
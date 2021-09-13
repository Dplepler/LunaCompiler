#ifndef LEXER_H
#define LEXER_H
#include "AST.h"
#define VALUE_SIZE 2

typedef struct LEXER_STRUCT
{
	token_T** tokens;
	size_t tokensSize;

	size_t index;
	size_t contentsLength;

	char* contents;
	char c;

}lexer_T;


void lexer_advance(lexer_T* lexer);
void lexer_skip_whitespace(lexer_T* lexer);
void lexer_skip_comments(lexer_T* lexer);
char lexer_peek(lexer_T* lexer, size_t offset);
lexer_T* init_lexer(char* contents);
token_T* lexer_token_peek(lexer_T* lexer, unsigned int offset);
token_T* lexer_get_next_token(lexer_T* lexer);
token_T* lexer_collect_id(lexer_T* lexer);
token_T* lexer_collect_number(lexer_T* lexer);
token_T* lexer_collect_string(lexer_T* lexer);
void lexer_token_list_push(lexer_T* lexer, token_T* token);
void lexer_free_tokens(lexer_T* lexer);


#endif

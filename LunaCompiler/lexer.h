#ifndef LEXER_H
#define LEXER_H
#include "AST.h"
#define VALUE_SIZE 2

typedef struct LEXER_STRUCT
{

	unsigned int index;
	unsigned int contentsLength;
	char* contents;
	char c;

}lexer_T;


void lexer_advance(lexer_T* lexer);
void lexer_skip_whitespace(lexer_T* lexer);
lexer_T* init_lexer(char* contents);
token_T* lexer_get_next_token(lexer_T* lexer);
token_T* lexer_collect_id(lexer_T* lexer);
token_T* lexer_collect_number(lexer_T* lexer);
token_T* lexer_collect_string(lexer_T* lexer);



#endif

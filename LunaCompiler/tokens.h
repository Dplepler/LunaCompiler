#ifndef TOKEN_H
#define TOKEN_H
#include "io.h"


typedef struct TOKEN_STRUCT
{
	char* value;
	enum
	{
		TOKEN_ID,
		TOKEN_NUMBER,
		TOKEN_EQUALS,
		TOKEN_DEQUAL,
		TOKEN_COMMA,
		TOKEN_STRING,
		TOKEN_SEMI,
		TOKEN_LPAREN,
		TOKEN_RPAREN,
		TOKEN_LESS,
		TOKEN_MORE,
		TOKEN_ELESS,
		TOKEN_EMORE,
		TOKEN_MUL,
		TOKEN_ADD,
		TOKEN_DIV,
		TOKEN_SUB,
		TOKEN_LBRACE,
		TOKEN_RBRACE,
		TOKEN_NOOP,
		TOKEN_EOF,
		
	} type;

} token_T;

typedef struct TOKEN_LIST
{
	token_T** tokens;
	size_t size;

}token_list;


token_T* init_token(int type, char* value);
token_list* init_token_list();
void token_list_push(token_list* list, token_T* token);


#endif

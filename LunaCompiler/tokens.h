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
		TOKEN_COMPARE,
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
		TOKEN_EOF,
		
	} type;

} token_T;

token_T* init_token(int type, char* value);

#endif
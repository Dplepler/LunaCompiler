#include "tokens.h"

/*
init_token initializes a token
Input: Type of token, value of token
Output: Token
*/
token_T* init_token(int type, char* value)
{
	token_T* token = calloc(1, sizeof(token_T));
	token->value = value;
	token->type = type;

	return token;
}



#include "tokens.h"

/*
init_token initializes a token
Input: Type of token, value of token
Output: Token
*/
token_T* init_token(int type, char* value)
{
	token_T* token = (token_T*)malloc(sizeof(token));
	token->value = value;
	token->type = type;

	return token;
}

token_list* init_token_list()
{
	token_list* list = calloc(1, sizeof(token_list));
	list->tokens = calloc(1, sizeof(token_T*));

	return list;
}

void token_list_push(token_list* list, token_T* token)
{
	list->tokens = realloc(list->tokens, sizeof(token_T*) * ++list->size);
	list->tokens[list->size - 1] = token;

}

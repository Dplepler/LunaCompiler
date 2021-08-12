#include <stdio.h>
#include <stdlib.h>
#include "token.h"

token_T* init_token(int type, char* value)
{
	token_T* token = (token_T*)calloc(1, sizeof(token_T));
	*token = { type, value };
}
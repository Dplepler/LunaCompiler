#include "lexer.h"

/*
init_lexer initializes the lexer with the source code (contents)
Input: Source code
Output: Lexer
*/
lexer_T* init_lexer(char* contents)
{
	lexer_T* lexer = (lexer_T*)calloc(1, sizeof(lexer));
	//lexer->list = init_token_list();

	lexer->contents = contents;
	lexer->contentsLength = strlen(contents);
	lexer->index = 0;
	lexer->c = contents[lexer->index];

	return lexer;
}

/*
lexer_advance advances the lexer by 1
Input: lexer
Output: None
*/
void lexer_advance(lexer_T* lexer)
{	
	if (lexer->index < lexer->contentsLength)
	{
		lexer->index++;
		lexer->c = lexer->contents[lexer->index];
	}
	else
		lexer->c = '\0';

}

char lexer_peek(lexer_T* lexer, size_t offset)
{
	char token = -1;

	if (lexer->index + 1 < lexer->contentsLength)
		token = lexer->contents[lexer->index + offset];

	return token;
}

/*
lexer_advance_current initializes a token with a type and a value
Input: Lexer, type of token
Output: Initialized token
*/
token_T* lexer_advance_current(lexer_T* lexer, int type)
{
	token_T* token = NULL;
	char* value = (char*)malloc(VALUE_SIZE);
	
	value[0] = lexer->c;

	// If token is 2 characters long (e.g: <=, ==)
	if (type == TOKEN_ELESS || type == TOKEN_EMORE || type == TOKEN_DEQUAL)
	{
		value = (char*)realloc(value, VALUE_SIZE + 1);
		lexer_advance(lexer);
		value[1] = lexer->c;
		value[2] = 0;
	}
	else
		value[1] = 0;
	
	token = init_token(type, value);
	lexer_advance(lexer);

	return token;
}

/*
lexer_skip_whitespace Skips all the unnecessary spaces and white space
Input: Lexer
Output: None
*/
void lexer_skip_whitespace(lexer_T* lexer)
{
	while (lexer->c == ' ' || lexer->c == '\n' || lexer->c == '\t')
		lexer_advance(lexer);
	
}

/*
lexer_get_next_token gets the next token and uses other functions to initialize a value and type
Input: Lexer
Output: The new token
*/
token_T* lexer_get_next_token(lexer_T* lexer)
{
	token_T* token = NULL;

	lexer_skip_whitespace(lexer);

	if (isalpha(lexer->c))
		token = lexer_collect_id(lexer);
	else if (isdigit(lexer->c))
		token = lexer_collect_number(lexer);
	else
	{
		switch (lexer->c)
		{
			case '=':
				if (lexer_peek(lexer, 1) == '=')
					token = lexer_advance_current(lexer, TOKEN_DEQUAL);
				else
					token = lexer_advance_current(lexer, TOKEN_EQUALS);
				break;

			case '<':
				if (lexer_peek(lexer, 1) == '=')
					token = lexer_advance_current(lexer, TOKEN_ELESS);
				else
					token = lexer_advance_current(lexer, TOKEN_LESS);
				break;

			case '>':
				if (lexer_peek(lexer, 1) == '=')
					token = lexer_advance_current(lexer, TOKEN_EMORE);
				else
					token = lexer_advance_current(lexer, TOKEN_MORE);
				break;

			case '"': token = lexer_collect_string(lexer); break;
			case ';': token = lexer_advance_current(lexer, TOKEN_SEMI); break;
			case '(': token = lexer_advance_current(lexer, TOKEN_LPAREN); break;
			case ')': token = lexer_advance_current(lexer, TOKEN_RPAREN); break;
			case '*': token = lexer_advance_current(lexer, TOKEN_MUL); break;
			case '+': token = lexer_advance_current(lexer, TOKEN_ADD); break;
			case '/': token = lexer_advance_current(lexer, TOKEN_DIV); break;
			case '-': token = lexer_advance_current(lexer, TOKEN_SUB); break;
			case ',': token = lexer_advance_current(lexer, TOKEN_COMMA); break;
			case '{': token = lexer_advance_current(lexer, TOKEN_LBRACE); break;
			case '}': token = lexer_advance_current(lexer, TOKEN_RBRACE); break;
			case '\0': token = lexer_advance_current(lexer, TOKEN_EOF);  break;
			default: printf("Unknown token: '%c'", lexer->c); exit(1);
		}
	}
	
	//token_list_push(lexer->list, token);
	return token;
}

/*
lexer_collect_id collects an identifier token
Input: Lexer
Output: Identifier token
*/
token_T* lexer_collect_id(lexer_T* lexer)
{
	char* id = (char*)malloc(sizeof(char));
	size_t size = 0;
	
	
	while (isalpha(lexer->c) || isdigit(lexer->c))
	{
		id = (char*)realloc(id, ++size);
		id[size - 1] = lexer->c;
		lexer_advance(lexer);
	}

	id[size] = 0;

	return init_token(TOKEN_ID, id);
}
 
/*
lexer_collect_number collects a number token
Input: Lexer
Output: Number token
*/
token_T* lexer_collect_number(lexer_T* lexer)
{

	char* num = (char*)malloc(sizeof(char));
	size_t size = 0;

	while (isdigit(lexer->c))
	{
		num = (char*)realloc(num, size + 1);
		num[size] = lexer->c;
		size++;
		lexer_advance(lexer);
	}
	
	num[size] = 0;

	return init_token(TOKEN_NUMBER, num);

}

/*
lexer_collect_string collects a string token
Input: Lexer
Output: String token
*/
token_T* lexer_collect_string(lexer_T* lexer)
{
	char* string = (char*)malloc(sizeof(char));
	size_t size = 0;

	lexer_advance(lexer);
	while (isalpha(lexer->c))
	{
		string = (char*)realloc(string, size + 1);
		string[size] = lexer->c;  
		size++;
		lexer_advance(lexer);
	}

	string[size] = 0;

	lexer_advance(lexer);

	return init_token(TOKEN_STRING, string);
}

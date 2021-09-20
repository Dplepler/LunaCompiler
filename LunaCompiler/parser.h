#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"

#define RESERVED_SIZE 7

typedef struct PARSER_STRUCT
{
	lexer_T* lexer;
	token_T* token;
	table_T* table;

	char** reserved;

	enum
	{
		OUT_T,
		IF_T,
		ELSE_T,
		WHILE_T,
		INT_T,
		STRING_T,
		RETURN_T,

	} reserved_T;


}parser_T;

parser_T* init_parser(lexer_T* lexer);

token_T* parser_expect(parser_T* parser, int type);

AST* parser_lib(parser_T* parser);
AST* parser_parse(parser_T* parser);
AST* parser_function(parser_T* parser);
AST* parser_block(parser_T* parser);
AST* parser_var_dec(parser_T* parser);
AST* parser_statement(parser_T* parser);
AST* parser_assignment(parser_T* parser);
AST* parser_func_call(parser_T* parser);
AST* parser_expression(parser_T* parser);
AST* parser_term(parser_T* parser);
AST* parser_factor(parser_T* parser);
AST* parser_int(parser_T* parser);
AST* parser_string(parser_T* parser);
AST* parser_id(parser_T* parser);
AST* parser_binary_expression(parser_T* parser);
AST* parser_condition(parser_T* parser);
AST* parser_while(parser_T* parser);
AST* parser_return(parser_T* parser);

char* reserved_to_string(int type);

int parser_check_reserved(parser_T* parser);


#endif

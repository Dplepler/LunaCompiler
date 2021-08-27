#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "symbolTable.h"

#define RESERVED_SIZE 5

 
typedef struct PARSER_STRUCT
{
	lexer_T* lexer;
	token_T* token;
	table_T* table;

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
AST* parser_functionCall(parser_T* parser);
AST* parser_expression(parser_T* parser);
AST* parser_term(parser_T* parser);
AST* parser_factor(parser_T* parser);
AST* parser_int(parser_T* parser);
AST* parser_id(parser_T* parser);
AST* parser_binary_expression(parser_T* parser);
AST* parser_condition(parser_T* parser);
AST* parser_while(parser_T* parser);
AST* parser_return(parser_T* parser);
int parser_check_reserved(parser_T* parser);


#endif

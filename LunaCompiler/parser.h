#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef struct PARSER_STRUCT
{
	lexer_T* lexer;
	token_T* token;

}parser_T;

parser_T* init_parser(lexer_T* lexer);
token_T* parser_expect(parser_T* parser, int type);
AST* parser_parse(parser_T* parser);
AST* parser_statement(parser_T* parser);
AST* parser_assignment(parser_T* parser);
AST* parser_functionCall(parser_T* parser);
AST* parser_expression(parser_T* parser);
AST* parser_term(parser_T* parser);
AST* parser_factor(parser_T* parser);


#endif

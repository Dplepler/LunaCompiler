#include "parser.h"

/*
init_parser initializes the parser
Input: Lexer
Output: Parser
*/
parser_T* init_parser(lexer_T* lexer)
{
	parser_T* parser = (parser_T*)malloc(sizeof(parser_T));
	parser->lexer = lexer;
	parser->token = lexer_get_next_token(parser->lexer);

	return parser;
}

/*
parser_expect advances the parser if the next token is matching the function's given token
Input: Parser, expected type
Output: The next token
*/
token_T* parser_expect(parser_T* parser, int type)
{
	token_T* token = NULL;

	if (parser->token->type == type)
	{
		parser->token = lexer_get_next_token(parser->lexer);		// If everything is okay, get and return the next token
	}
	else
	{
		printf("[ERROR]: Unexpected token\n");
		exit(1);													// Finish with error
	}

	return token;

}
/*
parser_parse is the main function of the parser which begins the parsing process
Input: Parser
Output: Root of the abstract syntax tree
*/
AST* parser_parse(parser_T* parser)
{
	AST* root =  parser_expression(parser);
	printTree(root);
	return root;
}

/*
parser_expression parses an expression which can be an addition or substruction of two expressions
Input: Parser
Output: AST node
*/
AST* parser_expression(parser_T* parser)
{
	AST* node = NULL;

	if (parser->token->type == TOKEN_ADD || parser->token->type == TOKEN_SUB)
	{
		parser->token = lexer_get_next_token(parser->lexer);
	}

	node = parser_term(parser);

	while (parser->token->type == TOKEN_ADD || parser->token->type == TOKEN_SUB)
	{

		if (parser->token->type == TOKEN_ADD)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip Add / Minus signs
			node = AST_opNode(node, parser_term(parser), AST_ADD);
		}
		if (parser->token->type == TOKEN_SUB)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip Add / Minus signs
			node = AST_opNode(node, parser_term(parser), AST_SUB);
		}	
	}
	return node;

}

/*
parser_term parses a term which is a multiplication or division of two expressions
Input: Parser
Output: AST node
*/
AST* parser_term(parser_T* parser)
{
	AST* node = parser_factor(parser);

	while (parser->token->type == TOKEN_MUL || parser->token->type == TOKEN_DIV)
	{
		if (parser->token->type == TOKEN_MUL)
		{  
			parser->token = lexer_get_next_token(parser->lexer);		// Skip multiplication / division signs
			node = AST_opNode(node, parser_factor(parser), AST_MUL);
		}
		if (parser->token->type == TOKEN_DIV)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip multiplication / division signs
			node = AST_opNode(node, parser_factor(parser), AST_DIV);
		}
	}

	return node;
}
/*
parser_factor parses a factor which can be an expression within parenthesis, a number or an identifier
Input: Parser
Output: AST node
*/
AST* parser_factor(parser_T* parser)
{
	AST* node = NULL;

	// If ( was found, parse an expression within the parenthesis
	if (parser->token->type == TOKEN_LPAREN)
	{
		parser->token = lexer_get_next_token(parser->lexer);	// Skip starting parenthesis
		node = parser_expression(parser);			// Parse expression
		parser_expect(parser, TOKEN_RPAREN);		// Skip the closing parenthesis
	}
	// Parse the number or identifier
	else if (parser->token->type == TOKEN_ID || parser->token->type == TOKEN_NUMBER)
	{
		node = init_AST(AST_INT);							
		node->int_value = atoi(parser->token->value);			// Copy token value into node
		parser->token = lexer_get_next_token(parser->lexer);	// Skip number
	}
	else
	{
		printf("[ERROR]: Syntax Error!\n");
		exit(1);
	}
	

	return node;
}
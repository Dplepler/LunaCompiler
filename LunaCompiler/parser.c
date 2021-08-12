#include "parser.h"

const char** reserved = { "int" };
enum
{
	INT_T,
}reserved_T;

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

	if (parser->token->type == type)
	{
		parser->token = lexer_get_next_token(parser->lexer);		// If everything is okay, get and return the next token
	}
	else
	{
		printf("[ERROR]: Unexpected token\n");
		exit(1);													// Finish with error
	}

	return parser->token;

}
/*
parser_parse is the main function of the parser which begins the parsing process
Input: Parser
Output: Root of the abstract syntax tree
*/
AST* parser_parse(parser_T* parser)
{
	AST* root =  parser_function(parser);
	printTree(root);
	return root;
}

AST* parser_lib(parser_T* parser)
{
	AST* root = NULL;
	do 
	{
		parser_function(parser);
	} while (parser->token->type != EOF);
}

AST* parser_function(parser_T* parser)
{
	AST* node = NULL;

	if (parser->token->type == TOKEN_LBRACE)
	{
		node = parser_block(parser);
	}
	else if (parser->token->type == TOKEN_ID)
	{
		if (!strcmp(parser->token->value, reserved[INT_T]))
		{

		}
		else
		{
			printf("[ERROR]: ")
		}
	}

}

AST* parser_block(parser_T* parser)
{
	parser->token = parser_expect(parser, TOKEN_LBRACE);
	while (parser->token->type != TOKEN_RBRACE)
	{

	}

}

AST* parser_statement(parser_T* parser)
{
	AST* node = NULL;

	if (parser->token->type == TOKEN_ID)
	{
		// Variable assignment
		if (lexer_peak(parser->lexer, 1) == '=')
		{
			node = parser_assignment(parser);
		}
		// Function call
		else
		{
			node = parser_functionCall(parser);
		} 
	}


	return node;
}

AST* parser_assignment(parser_T* parser)
{
	AST* node = init_AST(AST_VARIABLE);
	node->name = parser->token->value;

	parser->token = lexer_get_next_token(parser->lexer);
	parser->token = parser_expect(parser, TOKEN_EQUALS);

	node = AST_initChildren(node, parser_expression(parser), AST_ASSIGNMENT);

	return node;
}

AST* parser_functionCall(parser_T* parser)
{
	AST* node = init_AST(AST_FUNC_CALL);
	unsigned int counter = 0;

	node->name = parser->token->value;

	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->arguments = (AST**)malloc(sizeof(AST*));

	while (parser->token->type != TOKEN_RPAREN)
	{
		counter++;
		node->arguments = (AST**)realloc(node->arguments, sizeof(AST*) * counter);
		node->arguments[counter - 1] = parser_expression(parser);

		parser->token = lexer_get_next_token(parser->lexer);
		if (parser->token->type != TOKEN_RPAREN)
			parser->token = parser_expect(parser, TOKEN_COMMA);
	}

	node->argument_amount = counter;

	parser_expect(parser, TOKEN_RPAREN);

	return node;
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
			node = AST_initChildren(node, parser_term(parser), AST_ADD);
		}
		if (parser->token->type == TOKEN_SUB)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip Add / Minus signs
			node = AST_initChildren(node, parser_term(parser), AST_SUB);
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
			node = AST_initChildren(node, parser_factor(parser), AST_MUL);
		}
		if (parser->token->type == TOKEN_DIV)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip multiplication / division signs
			node = AST_initChildren(node, parser_factor(parser), AST_DIV);
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
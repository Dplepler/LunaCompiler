#include "parser.h"

const char* reserved[RESERVED_SIZE] = { "if", "else", "while", "int", "return" };
enum
{
	IF_T,
	ELSE_T,
	WHILE_T,
	INT_T,
	RETURN_T,


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
		printf("[ERROR]: Unexpected token\n Expected: %d, got: %d", type, parser->token->type);
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
	AST* root =  parser_expression(parser);
	
	//printf("Test: %s\n", root->function_list[0]->function_body->children[1]->value);
	//printTree(root);
	return root;
}

AST* parser_lib(parser_T* parser)
{
	AST* root = init_AST(AST_PROGRAM);
	size_t counter = 0;

	root->function_list = (AST**)malloc(sizeof(AST*));
	do 
	{
		root->function_list = (AST**)realloc(root->function_list, sizeof(AST*) * ++counter);
		root->function_list[counter--] = parser_function(parser);
	
	} while (parser->token->type != TOKEN_EOF);

	root->size = counter;

	return root;
}

AST* parser_function(parser_T* parser)
{
	AST* node = init_AST(AST_FUNCTION);
	size_t counter = 0;

	if (parser_check_reserved(parser) == INT_T)
	{
		node->function_return_type = parser->token->value;
	}
	else
	{
		printf("[ERROR]: Invalid return type\n");
		exit(1);
	}
		
	parser->token = parser_expect(parser, TOKEN_ID);
	  
	if (parser->token->type == TOKEN_ID)		// Check next token
		node->name = parser->token->value;
	
	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->function_def_args = (AST**)malloc(sizeof(AST*));

	while (parser->token->type != TOKEN_RPAREN)
	{
		node->function_def_args = (AST**)realloc(node->function_def_args, sizeof(AST*) * ++counter);
		node->function_def_args[counter--] = parser_var_dec(parser);

		if (parser->token->type != TOKEN_RPAREN)
			parser->token = parser_expect(parser, TOKEN_COMMA);
	}
	node->size = counter;

	parser->token = parser_expect(parser, TOKEN_RPAREN);
	node->function_body = parser_block(parser);

	return node;
}

AST* parser_block(parser_T* parser)
{
	AST* node = init_AST(AST_COMPOUND);
	size_t counter = 0;

	node->children = (AST**)malloc(sizeof(AST*));

	parser->token = parser_expect(parser, TOKEN_LBRACE);

	while (parser->token->type != TOKEN_RBRACE)
	{
		node->children = (AST**)realloc(node->children, sizeof(AST*) * ++counter);
		node->children[counter--] = parser_statement(parser);
	}
	node->size = counter;

	parser->token = parser_expect(parser, TOKEN_RBRACE);

	return node;
}

AST* parser_statement(parser_T* parser)
{
	AST* node = NULL;
	int type;

	// Checking all possible statement options
	if (parser->token->type == TOKEN_ID)
	{
		if ((type = parser_check_reserved(parser)) > -1)
		{
			switch (type)
			{
				case INT_T: node = parser_var_dec(parser); break;
				case IF_T: node = parser_condition(parser); break;
				case WHILE_T: node = parser_while(parser); break;
				case RETURN_T: node = parser_return(parser); break;
				
			}
			if (type >= INT_T)		// All of the reserved options after and including INT_T end with a semi colon
			{
				parser->token = parser_expect(parser, TOKEN_SEMI);

				if (type == RETURN_T)
					while (parser->token->type != TOKEN_RBRACE)					// Skipping all code after the return in the block, because it can never be reached
						parser->token = lexer_get_next_token(parser->lexer);
			}
		}
		// Variable assignment
		else if (lexer_peek(parser->lexer, 1) == '=')
		{
			node = parser_assignment(parser);
			parser->token = parser_expect(parser, TOKEN_SEMI);
		}
		else
		{
			node = parser_expression(parser);
		} 
	}


	return node;
}

AST* parser_assignment(parser_T* parser)
{
	AST* node = init_AST(AST_VARIABLE);
	node->name = parser->token->value;

	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_EQUALS);

	node = AST_initChildren(node, parser_expression(parser), AST_ASSIGNMENT);

	return node;
}

AST* parser_functionCall(parser_T* parser)
{
	AST* node = init_AST(AST_FUNC_CALL);
	size_t counter = 0;

	node->name = parser->token->value;
	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->arguments = (AST**)malloc(sizeof(AST*));

	while (parser->token->type != TOKEN_RPAREN)
	{
		node->arguments = (AST**)realloc(node->arguments, sizeof(AST*) * ++counter);
		node->arguments[counter - 1] = parser_expression(parser);

		if (parser->token->type != TOKEN_RPAREN)
			parser->token = parser_expect(parser, TOKEN_COMMA);
	}
	node->size = counter;

	parser_expect(parser, TOKEN_RPAREN);

	return node;
}

AST* parser_var_dec(parser_T* parser)
{
	AST* node = init_AST(AST_VARIABLE_DEC);
	if (parser->token->type == TOKEN_ID)
	{
		switch (parser_check_reserved(parser))
		{
			case INT_T: node->var_type = VAR_INT; break;
			default: printf("[ERROR]: Variable decleration missing variable type value\n");
				exit(1);
		}
	
	}

	parser->token = parser_expect(parser, TOKEN_ID);

	node->name = parser->token->value;

	parser->token = parser_expect(parser, TOKEN_ID);

	if (parser->token->type == TOKEN_EQUALS)
	{
		parser->token = lexer_get_next_token(parser->lexer);
		node->value = parser_expression(parser);
	}

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

	if (parser->token->type == TOKEN_ADD)
		parser->token = lexer_get_next_token(parser->lexer);

	node = parser_term(parser);

	while (parser->token->type == TOKEN_ADD || parser->token->type == TOKEN_SUB)
	{
		if (parser->token->type == TOKEN_ADD)
		{
			parser->token = lexer_get_next_token(parser->lexer);		// Skip Add / Minus signs
			node = AST_initChildren(node, parser_term(parser), AST_ADD);
		}
		else if (parser->token->type == TOKEN_SUB)
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
		else if (parser->token->type == TOKEN_DIV)
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

	switch (parser->token->type)
	{
		// If ( was found, parse an expression within the parenthesis
		case TOKEN_LPAREN:
			parser->token = lexer_get_next_token(parser->lexer);	// Skip starting parenthesis
			node = parser_expression(parser);						// Parse expression
			parser_expect(parser, TOKEN_RPAREN);					// Skip the closing parenthesis
			break;

		// Parse the number or identifier
		case TOKEN_NUMBER: node = parser_int(parser); break;
		case TOKEN_ID: node = parser_id(parser); break;
		// Case for unary operators (e.g: -6, -2 etc)
		case TOKEN_SUB: parser->token = lexer_get_next_token(parser->lexer);  node = AST_initChildren(0, parser_factor(parser), AST_SUB); break;

		default: printf("[ERROR]: Syntax Error!, Token: %s was unexpected\n", parser->token->value);
			exit(1);

	}

	return node;
}

AST* parser_int(parser_T* parser)
{
	AST* node = init_AST(AST_INT);
	node->int_value = parser->token->value;			// Copy token value into node
	parser->token = parser_expect(parser, TOKEN_NUMBER);	// Skip number

	return node;
}

AST* parser_id(parser_T* parser)
{
	AST* node = NULL;
	if (lexer_peek(parser->lexer, 1) == '(')
	{
		node = parser_functionCall(parser);
	}
	else
	{
		node = init_AST(AST_VARIABLE);
		node->name = parser->token->value;
		parser->token = parser_expect(parser, TOKEN_ID);
	}

	return node;
}

AST* parser_binary_expression(parser_T* parser)
{
	AST* node = init_AST(AST_COMPARE);

	node->leftChild = parser_expression(parser);	// Getting first expression

	if (parser->token->type == TOKEN_MORE || parser->token->type == TOKEN_EMORE || parser->token->type == TOKEN_LESS || parser->token->type == TOKEN_ELESS || parser->token->type == TOKEN_DEQUAL)
	{
		node->type_c = parser->token->type;
		parser->token = lexer_get_next_token(parser->lexer);
		node->rightChild = parser_expression(parser);
	}
	else
	{
		node->type_c = TOKEN_NOOP;
	}

	parser->token = parser_expect(parser, TOKEN_RPAREN);

	return node;
}

AST* parser_condition(parser_T* parser)
{
	AST* node = init_AST(AST_IF);
	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->condition = parser_binary_expression(parser);
	node->if_body = parser_block(parser);

	if (parser_check_reserved(parser) == ELSE_T)
	{
		node->else_body = parser_block(parser);
	}

	return node;
}
AST* parser_while(parser_T* parser)
{
	AST* node = init_AST(AST_WHILE);
	node->condition = parser_binary_expression(parser);
	node->if_body = parser_block(parser);

	return node;
}
AST* parser_return(parser_T* parser)
{
	AST* node = init_AST(AST_RETURN);

	parser->token = parser_expect(parser, TOKEN_ID);
	node->value = parser_expression(parser);

	return node;
}

int parser_check_reserved(parser_T* parser)
{
	int type = -1;
	unsigned int i = 0;
	bool found = false;

	if (parser->token->type == TOKEN_ID)
	{
		for (i = 0; i < RESERVED_SIZE && !found; i++)
		{
			if (!strcmp(parser->token->value, reserved[i]))
			{
				type = i;
				found = true;
			}
		}
	}
	
	return type;
}
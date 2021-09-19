#include "parser.h"

const char* reserved[RESERVED_SIZE] = { "print", "if", "else", "while", "int", "return" };

enum
{
	OUT_T,
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
	parser_T* parser = calloc(1, sizeof(parser_T));
	parser->lexer = lexer;
	parser->token = lexer_get_next_token(parser->lexer);

	parser->table = init_table(NULL);
	
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
		if (parser->token->type == TOKEN_ID)
			printf("[ERROR]: Missing token %s, got: %s", typeToString(type), parser->token->value);
		else
			printf("[ERROR]: Missing token %s, got: %s", typeToString(type), typeToString(parser->token->type));
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
	AST* root =  parser_lib(parser);
	return root;
}

AST* parser_lib(parser_T* parser)
{
	AST* root = init_AST(AST_PROGRAM);
	AST* node = NULL;
	size_t funcCounter = 0;
	size_t globalCounter = 0;

	root->function_list = calloc(1, sizeof(AST*));

	do 
	{
		node = parser_statement(parser);
		if (node->type == AST_FUNCTION)
		{
			root->function_list = realloc(root->function_list, sizeof(AST*) * ++funcCounter);
			root->function_list[funcCounter - 1] = node;
		}
		else
		{	
			root->children = realloc(root->children, sizeof(AST*) * ++globalCounter);
			root->children[globalCounter - 1] = node;
		}

	} while (parser->token->type != TOKEN_EOF);

	root->functionsSize = funcCounter;
	root->size = globalCounter;

	return root;
}

AST* parser_function(parser_T* parser)
{
	AST* node = init_AST(AST_FUNCTION);
	size_t counter = 0;

	switch (parser_check_reserved(parser))
	{
		case INT_T: node->var_type = DATA_INT; break;
		default: printf("[ERROR]: Bad function declaration\n"); 
			exit(1);
	}
		
	parser->token = parser_expect(parser, TOKEN_ID);
	  
	if (parser->token->type == TOKEN_ID)		// Give node the function name if it exists
		node->name = parser->token->value;
	
	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->function_def_args = calloc(1, sizeof(AST*));

	table_add_entry(parser->table, node->name, node->var_type);

	parser->table = table_add_table(parser->table);

	while (parser->token->type != TOKEN_RPAREN)
	{
		
		node->function_def_args = realloc(node->function_def_args, sizeof(AST*) * ++counter);
		node->function_def_args[counter - 1] = parser_var_dec(parser);

		if (parser->token->type != TOKEN_RPAREN)
			parser->token = parser_expect(parser, TOKEN_COMMA);
	}
	node->size = counter;

	parser->token = parser_expect(parser, TOKEN_RPAREN);
	node->function_body = parser_block(parser);

	parser->table = parser->table->prev;

	return node;
}

AST* parser_block(parser_T* parser)
{
	AST* node = init_AST(AST_COMPOUND);
	size_t counter = 0;

	node->children = calloc(1, sizeof(AST*));
	parser->token = parser_expect(parser, TOKEN_LBRACE);

	while (parser->token->type != TOKEN_RBRACE)
	{
		node->children = realloc(node->children, sizeof(AST*) * ++counter);
		node->children[counter - 1] = parser_statement(parser);
	}
	node->size = counter;

	parser->token = parser_expect(parser, TOKEN_RBRACE);

	return node;
}

AST* parser_statement(parser_T* parser)
{
	AST* node = NULL;
	int type = 0;

	// Checking all possible statement options
	if (parser->token->type == TOKEN_ID)
	{
		if ((type = parser_check_reserved(parser)) > -1)
		{
			switch (type)
			{
				case INT_T: 
					if (lexer_token_peek(parser->lexer, 2)->type == TOKEN_LPAREN)
						node = parser_function(parser);
					else
						node = parser_var_dec(parser); 
					break;

				case IF_T: node = parser_condition(parser); break;
				case WHILE_T: node = parser_while(parser); break;
				case RETURN_T: node = parser_return(parser); break;

			}
			if (node)
			{
				if (node->type == AST_VARIABLE_DEC || node->type == AST_RETURN)
				{
					parser->token = parser_expect(parser, TOKEN_SEMI);

					if (type == RETURN_T)
						while (parser->token->type != TOKEN_RBRACE)					// Skipping all code after the return in the block, because it can never be reached
							parser->token = lexer_get_next_token(parser->lexer);
				}
			}
		}
		// Variable assignment
		else if (lexer_token_peek(parser->lexer, 1)->type == TOKEN_EQUALS)
		{
			node = parser_assignment(parser);
			parser->token = parser_expect(parser, TOKEN_SEMI);
		}
		else if (lexer_token_peek(parser->lexer, 1)->type == TOKEN_LPAREN)
		{
			node = parser_func_call(parser);
			parser->token = parser_expect(parser, TOKEN_SEMI);
		}
		else
		{
			node = parser_expression(parser);
		} 
	}
	else if (parser->token->type == TOKEN_LBRACE)
	{
		parser->table = table_add_table(parser->table);
		node = parser_block(parser);
		parser->table = parser->table->prev;				// Exit current table and move to parent table
	}
	else if (parser->token->type == TOKEN_NUMBER || parser->token->type == TOKEN_ID)
	{
		node = parser_expression(parser);
		parser->token = parser_expect(parser, TOKEN_SEMI);
	}
	else
	{
		printf("[ERROR]: Invalid syntax\n"); exit(1);
	}

	return node;
}

AST* parser_assignment(parser_T* parser)
{
	AST* node = parser_id(parser);		// First make a variable node
	AST* reset = NULL;

	// Now assign that variable to be the left child of an assignment node, including an expression
	if (parser->token->type == TOKEN_EQUALS)
	{
		parser->token = lexer_get_next_token(parser->lexer);
		node = AST_initChildren(node, parser_expression(parser), AST_ASSIGNMENT);
	}
	// If variable was written without an assignment, reset it
	else
	{
		reset = init_AST(AST_INT);
		reset->int_value = "0";
		node = AST_initChildren(node, reset, AST_ASSIGNMENT);		
	}
}

AST* parser_func_call(parser_T* parser)
{
	AST* node = init_AST(AST_FUNC_CALL);
	size_t counter = 0;

	node->name = parser->token->value;
	parser->token = parser_expect(parser, TOKEN_ID);
	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->arguments = calloc(1, sizeof(AST*));

	while (parser->token->type != TOKEN_RPAREN)
	{
		node->arguments = realloc(node->arguments, sizeof(AST*) * ++counter);
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
			case INT_T: node->var_type = DATA_INT; break;
			default: printf("[ERROR]: Variable decleration missing variable type value\n");
				exit(1);
		}
	}

	parser->token = parser_expect(parser, TOKEN_ID);

	node->name = parser->token->value;
	table_add_entry(parser->table, node->name, node->var_type);

	node->value = parser_assignment(parser);

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

	if (lexer_token_peek(parser->lexer, 1)->type == TOKEN_LPAREN)
	{
		node = parser_func_call(parser);
	}
	else
	{
		node = init_AST(AST_VARIABLE);
		node->name = parser->token->value;
		parser->token = parser_expect(parser, TOKEN_ID);

		if (!table_search_entry(parser->table, node->name))
		{
			printf("Variable %s was not declared in the current scope\n", node->name);
			exit(1);
		}
	}

	return node;
}

AST* parser_binary_expression(parser_T* parser)
{
	AST* node = init_AST(AST_COMPARE);

	parser->token = parser_expect(parser, TOKEN_LPAREN);

	node->leftChild = parser_expression(parser);	// Getting first expression

	if (parser->token->type == TOKEN_MORE || parser->token->type == TOKEN_EMORE || parser->token->type == TOKEN_LESS 
		|| parser->token->type == TOKEN_ELESS || parser->token->type == TOKEN_DEQUAL || parser->token->type == TOKEN_NEQUAL)
	{
		node->type_c = parser->token->type;
		parser->token = lexer_get_next_token(parser->lexer);
		node->rightChild = parser_expression(parser);
	}
	else
	{
		node->value = node->leftChild;		// If there's one expression we switch it from left child to value field
		node->leftChild = NULL;
		node->type_c = TOKEN_NOOP;
	}
	parser->token = parser_expect(parser, TOKEN_RPAREN);

	return node;
}

AST* parser_condition(parser_T* parser)
{
	AST* node = init_AST(AST_IF);
	node->name = parser->token->value;

	parser->token = parser_expect(parser, TOKEN_ID);	

	node->condition = parser_binary_expression(parser);

	if (parser->token->type == TOKEN_LBRACE)
		node->if_body = parser_statement(parser);
	else
	{
		printf("[ERROR]: If statement missing braces\n");
		exit(1);
	}
		

	if (parser_check_reserved(parser) == ELSE_T)
	{
		parser->token = lexer_get_next_token(parser->lexer);

		if (parser->token->type == TOKEN_LBRACE)
			node->else_body = parser_statement(parser);
		else
		{
			printf("[ERROR]: else statement missing braces\n");
			exit(1);
		}
			
	}

	return node;
}
AST* parser_while(parser_T* parser)
{
	AST* node = init_AST(AST_WHILE);
	parser->token = lexer_get_next_token(parser->lexer);

	node->condition = parser_binary_expression(parser);

	if (parser->token->type == TOKEN_LBRACE)
		node->if_body = parser_statement(parser);
	else
	{
		printf("[ERROR]: While statement missing braces\n");
		exit(1);
	}
	
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

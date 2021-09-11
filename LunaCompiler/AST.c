#include "AST.h"

/*
init_AST initializes the abstract syntax tree
Input: Type of node
Output: None
*/
AST* init_AST(int type)
{
	AST* node = calloc(1, sizeof(AST));
	node->type = type;

	return node;

}

/*
AST_initChildren initializes an AST node of type any type and assigns given children to it
Input: Left node, right node, type of node
Output: New node
*/
AST* AST_initChildren(AST* left, AST* right, int type)
{
	AST* node = init_AST(type);
	node->leftChild = left;
	node->rightChild = right;

	return node;
}


/*
typeToString converts an AST node type to string so I can debug with it
Input: Type
Output: String value of type
*/
char* typeToString(int type)
{
	switch (type)
	{

		// TOKEN ENUM

		case TOKEN_ID: return "Identifier";
		case TOKEN_NUMBER: return "Number";
		case TOKEN_EQUALS: return "=";
		case TOKEN_DEQUAL: return "==";
		case TOKEN_COMMA: return ",";
		case TOKEN_STRING: return "String";
		case TOKEN_SEMI: return ";";
		case TOKEN_LPAREN: return "(";
		case TOKEN_RPAREN: return ")";
		case TOKEN_LESS: return "<";
		case TOKEN_MORE: return ">";
		case TOKEN_ELESS: return "<=";
		case TOKEN_EMORE: return ">=";
		case TOKEN_MUL: return "*";
		case TOKEN_ADD: return "+";
		case TOKEN_DIV: return "/";
		case TOKEN_SUB: return "-";
		case TOKEN_LBRACE: return "{";
		case TOKEN_RBRACE: return "}";
		case TOKEN_NOOP: return "NOOP";
		case TOKEN_EOF: return "End of file";

		// AST ENUM
	
		case AST_PROGRAM: return "Program";
		case AST_FUNCTION: return "Function";
		case AST_DEF_AMOUNT: return "Definition amount";
		case AST_INT: return "Int";
		case AST_ADD: return "ADD";
		case AST_SUB: return "SUB";
		case AST_MUL: return "MUL";
		case AST_DIV: return "DIV";
		case AST_ASSIGNMENT: return "Assignment";
		case AST_VARIABLE_DEC: return "Declaration";
		case AST_VARIABLE: return "Variable";
		case AST_FUNC_CALL: return "Function Call";
		case AST_PARAM: return "Param";
		case AST_IF: return "If";
		case AST_IFZ: return "If false";
		case AST_LABEL: return "Label";
		case AST_GOTO: return "Goto";
		case AST_WHILE: return "While";
		case AST_COMPARE: return "Comparsion";
		case AST_RETURN: return "Return";

		// Types
		case DATA_INT: return "Int";

	}
}

void AST_free_AST(AST* node)
{
	unsigned int i = 0;

	if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV
		|| node->type == AST_COMPARE || node->type == AST_ASSIGNMENT)
	{
		if (node->value)
		{
			AST_free_AST(node->value);
		}
		else
		{
			AST_free_AST(node->leftChild);
			AST_free_AST(node->rightChild);
		}

		free(node);
	}
	else if (node->type == AST_INT || node->type == AST_VARIABLE)
	{
		free(node);
	}
	else if (node->type == AST_IF || node->type == AST_WHILE)
	{
		AST_free_AST(node->if_body);
		AST_free_AST(node->condition);
		if (node->else_body)
			AST_free_AST(node->else_body);
		free(node);
	}
	else if (node->type == AST_VARIABLE_DEC || node->type == AST_RETURN)
	{
		AST_free_AST(node->value);
		free(node);
	}
	else
	{
		switch (node->type)
		{
			case AST_PROGRAM:
				// Freeing Functions
				for (i = 0; i < node->functionsSize; i++)
					AST_free_AST(node->function_list[i]);
				// Freeing global variables
				for (i = 0; i < node->size; i++)
					AST_free_AST(node->children[i]);

				if (node->function_list)
					free(node->function_list);
				if (node->children)
					free(node->children);
				free(node);
				break;

			case AST_FUNCTION:
				// Freeing Functions
				for (i = 0; i < node->size; i++)
					AST_free_AST(node->function_def_args[i]);

				if (node->function_def_args)
					free(node->function_def_args);
				if (node->function_body)
					AST_free_AST(node->function_body);
				
					

				free(node);
				break;

			case AST_COMPOUND:
				for (i = 0; i < node->size; i++)
					AST_free_AST(node->children[i]);
				if (node->children)
					free(node->children);
				free(node);
				break;

			case AST_FUNC_CALL:
				for (i = 0; i < node->size; i++)
					AST_free_AST(node->arguments[i]);
				if (node->arguments)
					free(node->arguments);

				free(node);
				break;

		}
	}
}

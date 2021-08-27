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
		case AST_INT: return "Int";
		case AST_ADD: return "Addition";
		case AST_SUB: return "Substruction";
		case AST_MUL: return "Multiplication";
		case AST_DIV: return "Division";
		case AST_ASSIGNMENT: return "Assignment";
		case AST_VARIABLE_DEC: return "Declaration";
		case AST_VARIABLE: return "Variable";
		case AST_FUNC_CALL: return "Function Call";
		case AST_IF: return "If";
		case AST_WHILE: return "While";
		case AST_COMPARE: return "Comparsion";
		case AST_RETURN: return "Return";

		
		

	}
}

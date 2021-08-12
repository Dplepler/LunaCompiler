#ifndef AST_H
#define AST_H
#include "tokens.h"

typedef struct AST_STRUCT
{

	enum
	{
		AST_COMPOUND,		
		AST_INT,
		AST_ADD,
		AST_SUB,
		AST_MUL,
		AST_DIV,
		AST_ASSIGNMENT,
		AST_VARIABLE,
		AST_FUNC_CALL,


	}type;

	// Name for different types of nodes
	char* name;

	// Int node
	int int_value;

	// Function call
	AST** arguments;
	int argument_amount;


	struct AST_STRUCT* leftChild;
	struct AST_STRUCT* value;
	struct AST_STRUCT* rightChild;

		

}AST;

AST* init_AST(int type);
AST* AST_initChildren(AST* left, AST* right, int type);
void printTree(AST* root);
void printLevel(AST* node, int level);
int getLevelCount(AST* node);
char* typeToString(int type);
#endif

#ifndef AST_H
#define AST_H
#include "tokens.h"

typedef struct AST_STRUCT
{

	enum
	{
		AST_PROGRAM,
		AST_FUNCTION,
		AST_COMPOUND,
		AST_INT,
		AST_ADD,
		AST_SUB,
		AST_MUL,
		AST_DIV,
		AST_ASSIGNMENT,
		AST_VARIABLE_DEC,
		AST_VARIABLE,
		AST_FUNC_CALL,
		AST_IF,
		AST_WHILE,
		AST_COMPARE,
		AST_RETURN,

	} type;

	// Compound
	struct AST_STRUCT** children;

	// Branch
	struct AST_STRUCT* condition;
	struct AST_STRUCT* if_body;
	struct AST_STRUCT* else_body;
	int type_c;							// Type of condition (>, ==, <=)


	// Variable decleration
	enum
	{
		VAR_INT,

	} var_type;

	// Functions
	struct AST_STRUCT** function_list;
	struct AST_STRUCT** function_def_args;
	struct AST_STRUCT* function_body;
	char* function_return_type;

	// Name for different types of nodes
	char* name;

	// Int node
	int int_value;

	// Function call
	struct AST_STRUCT** arguments;
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

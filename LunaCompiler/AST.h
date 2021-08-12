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

	}type;

	// If node is an integer
	int int_value;

	struct AST_STRUCT* leftChild;
	struct AST_STRUCT* rightChild;


}AST;

AST* init_AST(int type);
AST* AST_opNode(AST* left, AST* right, int type);
void printTree(AST* root);
void printLevel(AST* node, int level);
int getLevelCount(AST* node);
char* typeToString(int type);
#endif

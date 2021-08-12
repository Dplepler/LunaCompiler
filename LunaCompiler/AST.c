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
Initializes an AST node of type operation so I can assign a left and right child to it
Input: Left node, right node, type of operation
Output: New operation node
*/
AST* AST_opNode(AST* left, AST* right, int type)
{
	AST* node = init_AST(type);
	node->leftChild = left;
	node->rightChild = right;

	return node;
}

/*
printTree prints a binary tree
Input: Root of tree
Output: None
*/
void printTree(AST* root)
{
	int i = 0;
	int maxLevel = getLevelCount(root);
	for (i = 0; i < maxLevel; i++)
	{
		printLevel(root, i);
		printf("\n");

	}
	
}

/*
printLevel recursively prints a level of a binary tree
Input: A binary node, Level I want to print
Output: None
*/
void printLevel(AST* node, int level)
{
	if (node && !level)
	{

		if (node->type == AST_INT)
			printf("%d ", node->int_value);
		else
			printf("%s ", typeToString(node->type));
	}
	else if (node)
	{
		printLevel(node->leftChild, level - 1);
		printLevel(node->rightChild, level - 1);
	}
}

/*
getLevelCount recursively gets the maximum level count of a binary tree given to it
Input: The root of the tree
Output: Maximum level
*/
int getLevelCount(AST* node)
{
	int leftMaxLevel = 0;
	int rightMaxLevel = 0;

	if (!node)
		return 0;

	leftMaxLevel = 1 + getLevelCount(node->leftChild);
	rightMaxLevel = 1 + getLevelCount(node->rightChild);

	if (leftMaxLevel > rightMaxLevel)
		return leftMaxLevel;
	else
		return rightMaxLevel;
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
		case AST_COMPOUND: return "COMPOUND";
		case AST_INT: return "INT";
		case AST_ADD: return "ADDITION";
		case AST_SUB: return "SUBSTRUCTION";
		case AST_MUL: return "MULTIPLICATION";
		case AST_DIV: return "Division";

	}
}

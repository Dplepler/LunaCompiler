#include "TAC.h"

TAC_list* init_tac_list()
{
	TAC_list* list = (TAC_list*)calloc(1, sizeof(TAC_list));
	list->instructions = (TAC**)calloc(1, sizeof(TAC*));

	return list;
}

void list_push(TAC_list* list, TAC* instruction)
{
	list->instructions = (TAC**)realloc(list->instructions, sizeof(TAC*) * ++list->size);
	list->instructions[list->size--] = instruction;
}

TAC** traversal_visit(AST* node)
{
	TAC_list* list = init_tac_list();

	if (node)
	{
		traversal_build_instruction(node, list);
	}

	return list->instructions;
}

void traversal_build_instruction(AST* node, TAC_list* list)
{
	TAC* instruction = NULL;
	unsigned int i = 0;

	if (node)
	{
		if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV)
		{
			traversal_binop(node, list);
		}
		switch (node->type)
		{
			case AST_PROGRAM: 
				for (i = 0; i < node->size; i++)
					traversal_build_instruction(node->function_list[i], list);
				break;
			case AST_FUNCTION: 
		}
	}
	
}

void* traversal_build_argument(AST* node, TAC_list* list)
{
	void* arg = NULL;

	if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV)
	{
		arg = traversal_binop(node, list);	// Arg will be a temporary result (which is a pointer to an instruction in the array)
	}
	else if (node->type == AST_INT)
	{
		arg = (char*)node->int_value;
	}
	else if (node->type == AST_VARIABLE)
	{
		arg = (char*)node->name;
	}

	return arg;
}

TAC* traversal_binop(AST* node, TAC_list* list)
{
	TAC* instruction = (TAC*)malloc(sizeof(TAC));
 
	instruction->op = node->type;

	instruction->arg1 = traversal_build_argument(node->leftChild, list);
	instruction->arg2 = traversal_build_argument(node->rightChild, list);

	list_push(list, instruction);
	
	return instruction;
}




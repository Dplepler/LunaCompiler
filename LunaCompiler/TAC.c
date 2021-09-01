#include "TAC.h"

TAC_list* init_tac_list()
{
	TAC_list* list = calloc(1, sizeof(TAC_list));
	list->head = calloc(1, sizeof(TAC));
	list->last = calloc(1, sizeof(TAC));

	return list;
}

void list_push(TAC_list* list, TAC* instruction)
{
	if (!list->size)
		list->head = instruction;

	list->last->next = instruction;
	list->last = instruction;

	list->size++;
}

TAC_list* traversal_visit(AST* node)
{
	TAC_list* list = init_tac_list();

	traversal_build_instruction(node, list);
	
	return list;
}

void* traversal_build_instruction(AST* node, TAC_list* list)
{
	void* instruction = NULL;
	unsigned int i = 0;

	if (node)
	{
		if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV ||
			node->type == AST_COMPARE)
		{
			instruction = traversal_binop(node, list);
		}
		else
		{
			switch (node->type)
			{
				case AST_PROGRAM: 
					for (i = 0; i < node->size; i++)		// Loop through functions
						instruction = traversal_build_instruction(node->function_list[i], list);
					break;
				case AST_FUNCTION: instruction = traversal_func_dec(node, list); traversal_statements(node->function_body, list); break;
				case AST_COMPOUND: traversal_statements(node, list); break;
				case AST_ASSIGNMENT: instruction = traversal_assignment(node, list); break;
				case AST_VARIABLE_DEC: instruction = traversal_assignment(node->value, list); break;	// Variable declerations will become normal assignments
				case AST_FUNC_CALL: instruction = traversal_function_call(node, list); break;
				case AST_INT: instruction = (char*)node->int_value; break;
				case AST_VARIABLE: instruction = (char*)node->name; break;
				case AST_IF: traversal_if(node, list); break;
					
			}
		}
		
	}

	return instruction;
}


TAC* traversal_func_dec(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	// In this triple, arg1 will be the function name and arg2 will be the function return type
	instruction->arg1 = (char*)node->name;
	instruction->arg2 = (char*)node->function_return_type;

	instruction->op = node->type;

	list_push(list, instruction);

	return instruction;
}
/*
traversal_binop handles binary operations between two expressions
Input: An AST node, Triple list
Output: A triple
*/
TAC* traversal_binop(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	if (node->type == AST_COMPARE)
		instruction->op = node->type_c;
	else
		instruction->op = node->type;
	
	instruction->arg1 = traversal_build_instruction(node->leftChild, list);
	instruction->arg2 = traversal_build_instruction(node->rightChild, list);

	list_push(list, instruction);

	return instruction;
}

TAC* traversal_assignment(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = node->type;

	instruction->arg1 = node->leftChild->name;
	instruction->arg2 = traversal_build_instruction(node->rightChild, list);

	list_push(list, instruction);

	return instruction;
}

TAC* traversal_function_call(AST* node, TAC_list* list)
{
	TAC* instruction = NULL;
	unsigned int i = 0;
	size_t counter = 0;
	size_t size = 0;

	// Push params for function call
	for (i = 0; i < node->size; i++)
	{
		instruction = calloc(1, sizeof(TAC));
		instruction->arg1 = (char*)node->arguments[i]->name;
		instruction->op = AST_PARAM;
		list_push(list, instruction);
	}
	
	instruction = calloc(1, sizeof(TAC));

	instruction->op = node->type;
	instruction->arg1 = (char*)node->name;

	counter = node->size;
	while (counter)
	{
		counter /= 10;
		size++;
	}
	instruction->arg2 = (char*)calloc(1, size);
	_itoa(node->size, instruction->arg2, 10);

	list_push(list, instruction);

	return instruction;

}

void traversal_if(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));
	TAC* gotoInstruction = NULL;
	TAC* label = NULL;

	unsigned int i = 0;

	instruction->op = AST_IFZ;		// If false (If zero)

	if (node->condition->type_c == TOKEN_NOOP)		// If there's no relation operation (<, >, <= etc)
		instruction->arg1 = traversal_build_instruction(node->condition->value, list);
	else
		instruction->arg1 = traversal_binop(node->condition, list);

	list_push(list, instruction);
	
	for (i = 0; i < node->if_body->size; i++)
		traversal_build_instruction(node->if_body->children[i], list);

	
	label = calloc(1, sizeof(TAC));
	
	label->op = AST_LABEL;
	
	if (node->else_body)
	{
		gotoInstruction = calloc(1, sizeof(TAC));

		gotoInstruction->op = AST_GOTO;
		list_push(list, gotoInstruction);

		instruction->arg2 = traversal_build_instruction(node->else_body->children[0], list);		// Assign the goto of if to the start of the else
		for (i = 1; i < node->else_body->size; i++)
			traversal_build_instruction(node->else_body->children[i], list);

		list_push(list, label);
		gotoInstruction->arg1 = label;
	}
	else
	{
		list_push(list, label);
		instruction->arg2 = label;
	}	
}

void traversal_statements(AST* node, TAC_list* list)
{
	unsigned int i = 0;

	// For all the statements in the block, push them into the list
	for (i = 0; i < node->size; i++)
	{
		traversal_build_instruction(node->children[i], list);
	}

}

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
	list->instructions[list->size - 1] = instruction;
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
		if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV)
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
					
			}
		}
		
	}

	return instruction;
}


TAC* traversal_func_dec(AST* node, TAC_list* list)
{
	TAC* instruction = (TAC*)calloc(1, sizeof(TAC));

	// In this triple, arg1 will be the function name and arg2 will be the function return type
	instruction->arg1 = (char*)node->name;
	instruction->arg2 = (char*)node->function_return_type;

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
	TAC* instruction = (TAC*)calloc(1, sizeof(TAC));

	instruction->op = node->type;
	
	instruction->arg1 = traversal_build_instruction(node->leftChild, list);
	instruction->arg2 = traversal_build_instruction(node->rightChild, list);

	list_push(list, instruction);

	return instruction;
}

TAC* traversal_assignment(AST* node, TAC_list* list)
{
	TAC* instruction = (TAC*)calloc(1, sizeof(TAC));

	instruction->op = node->type;

	instruction->arg1 = node->leftChild->name;
	instruction->arg2 = traversal_build_instruction(node->rightChild, list);

	list_push(list, instruction);

	return instruction;
}

TAC* traversal_function_call(AST* node, TAC_list* list)
{
	unsigned int i = 0;
	TAC* instruction = NULL; 

	// Push params for function call
	for (i = 0; i < node->size; i++)
	{
		instruction = (TAC*)calloc(1, sizeof(TAC));
		instruction->arg1 = (char*)node->arguments[i]->name;
		list_push(list, instruction);
	}

	instruction = (TAC*)calloc(1, sizeof(TAC));
	printf("Node type: %s\n", typeToString(node->type));
	instruction->op = node->type;
	instruction->arg1 = node->name;
	instruction->arg2 = node->size;

	list_push(list, instruction);

	return instruction;

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
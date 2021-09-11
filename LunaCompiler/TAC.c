#include "TAC.h"

TAC_list* init_tac_list()
{
	TAC_list* list = calloc(1, sizeof(TAC_list));

	return list;
}

arg_T* init_arg(void* arg, int type)
{
	arg_T* argument = calloc(1, sizeof(arg_T));
	argument->value = arg;
	argument->type = type;

	return argument;
}

int traversal_check_arg(AST* node)
{
	if (node->type == AST_VARIABLE || node->type == AST_INT)
		return CHAR_P;
	else
		return TAC_P;
}

void list_push(TAC_list* list, TAC* instruction)
{
	if (!list->size)
		list->head = instruction;
	else
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
					for (i = 0; i < node->size; i++)	// Loop through global variables
						instruction = traversal_build_instruction(node->children[i], list);
					
					for (i = 0; i < node->functionsSize; i++)		// Loop through functions
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
				case AST_WHILE: traversal_while(node, list); break;
				case AST_RETURN: traversal_return(node, list); break;
					
			}
		}
		
	}

	return instruction;
}


TAC* traversal_func_dec(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));
	TAC* defAmount = calloc(1, sizeof(TAC));
	char* value = NULL;

	// In this triple, arg1 will be the function name and arg2 will be the function return type
	instruction->arg1 = init_arg(node->name, CHAR_P);
	instruction->arg2 = init_arg(typeToString(node->var_type), CHAR_P);
	
	instruction->op = node->type;

	list_push(list, instruction);

	defAmount->op = AST_DEF_AMOUNT;
	value = _itoa(node->size, (char*)calloc(1, numOfDigits(node->size) + 1), 10);
	defAmount->arg1 = init_arg(value, CHAR_P);
	
	list_push(list, defAmount);

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

	instruction->arg1 = init_arg(traversal_build_instruction(node->leftChild, list), traversal_check_arg(node->leftChild));
	instruction->arg2 = init_arg(traversal_build_instruction(node->rightChild, list), traversal_check_arg(node->rightChild));

	list_push(list, instruction);

	return instruction;
}

TAC* traversal_assignment(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = node->type;
	instruction->arg1 = init_arg(node->leftChild->name, CHAR_P);

	instruction->arg2 = init_arg(traversal_build_instruction(node->rightChild, list), traversal_check_arg(node->rightChild));

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
		instruction->arg1 = init_arg(node->arguments[i]->name, CHAR_P);
		instruction->op = AST_PARAM;
		list_push(list, instruction);
	}
	
	instruction = calloc(1, sizeof(TAC));

	instruction->op = node->type;
	instruction->arg1 = init_arg(node->name, CHAR_P);

	
	// Arg2 will be the number of arguments passing into the function
	instruction->arg2 = init_arg(calloc(1, numOfDigits(node->size) + 1), CHAR_P);
	_itoa(node->size, instruction->arg2->value, 10);

	list_push(list, instruction);

	return instruction;

}

TAC* traversal_condition(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = AST_IFZ;		// If false (If zero)

	if (node->type_c == TOKEN_NOOP)		// If there's no relation operation (<, >, <= etc)
		instruction->arg1 = init_arg(traversal_build_instruction(node->value, list), traversal_check_arg(node->value));
	else
		instruction->arg1 = init_arg(traversal_binop(node, list), TAC_P);

	list_push(list, instruction);

	return instruction;
}

void traversal_if(AST* node, TAC_list* list)
{
	TAC* instruction = traversal_condition(node->condition, list);
	TAC* label = calloc(1, sizeof(TAC));
	TAC* gotoInstruction = NULL;

	unsigned int i = 0;
	
	for (i = 0; i < node->if_body->size; i++)
		traversal_build_instruction(node->if_body->children[i], list);
	
	label->op = AST_LABEL;
	
	if (node->else_body)
	{
		gotoInstruction = calloc(1, sizeof(TAC));

		gotoInstruction->op = AST_GOTO;
		list_push(list, gotoInstruction);

		// Assign the goto of if to the start of the else
		instruction->arg2 = init_arg(traversal_build_instruction(node->else_body->children[0], list), TAC_P);		
		for (i = 1; i < node->else_body->size; i++)
			traversal_build_instruction(node->else_body->children[i], list);

		list_push(list, label);
		gotoInstruction->arg1 = init_arg(label, TAC_P);
	}
	else
	{
		list_push(list, label);
		instruction->arg2 = init_arg(label, TAC_P);
	}	
}

void traversal_while(AST* node, TAC_list* list)
{
	TAC* condition = traversal_condition(node->condition, list);		// Create a condition
	TAC* gotoInstruction = calloc(1, sizeof(TAC));
	TAC* label = calloc(1, sizeof(TAC));
	
	traversal_statements(node->if_body, list);		// Travel through the body of the while loop and create instructions

	// Jump to the start of the loop including the condition
	gotoInstruction->op = AST_GOTO;
	gotoInstruction->arg1 = init_arg(condition->arg1->value, TAC_P);	
	list_push(list, gotoInstruction);

	label->op = AST_LABEL;
	list_push(list, label);

	condition->arg2 = init_arg(label, TAC_P);

}

TAC* traversal_return(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = AST_RETURN;
	instruction->arg1 = init_arg(traversal_build_instruction(node->value, list), traversal_check_arg(node->value));

	list_push(list, instruction);

	return instruction;
}

void traversal_statements(AST* node, TAC_list* list)
{
	unsigned int i = 0;

	// These instructions are used to pass the information of when a block begins and ends
	TAC* start = calloc(1, sizeof(TAC));
	TAC* end = calloc(1, sizeof(TAC));

	start->op = TOKEN_LBRACE;
	list_push(list, start);
	end->op = TOKEN_RBRACE;

	// For all the statements in the block, push them into the list
	for (i = 0; i < node->size; i++)
	{
		traversal_build_instruction(node->children[i], list);
	}
	
	list_push(list, end);
}

void traversal_print_instructions(TAC_list* instructions)
{
	unsigned int i = 0;
	TAC* triple = instructions->head;

	for (i = 0; i < instructions->size; i++)
	{
		if (triple->op)
			printf("Operation: %s, ", typeToString(triple->op));
		if (triple->arg1)
		{
			if (triple->arg1->type == CHAR_P)
				printf("Arg1: %s, ", triple->arg1->value);
			else
				printf("Arg1: %p, ", triple->arg1->value);
		}
		if (triple->arg2)
		{
			if (triple->arg2->type)
				printf("Arg2: %s, ", triple->arg2->value);
			else
				printf("Arg2: %p, ", triple->arg2->value);
		}

		printf("Address: %p\n", triple);

		triple = triple->next;
	}
}




void traversal_free_array(TAC_list* list)
{
	TAC* triple = list->head;
	TAC* prev = NULL;

	while (triple->next)
	{
		if (triple->op == AST_FUNC_CALL)
		{
			free(triple->arg2->value);
		}
		if (triple->op == AST_DEF_AMOUNT)
		{
			free(triple->arg1->value);
		}
		
		
		if (triple->arg1)
			free(triple->arg1);
	
		if (triple->arg2)
			free(triple->arg2);

		prev = triple;
		triple = triple->next;
		free(prev);
	}
	free(list->last);
	free(list);
}

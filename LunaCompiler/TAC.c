#include "TAC.h"

/*
init_tac_list initializes a three address code list
Input: None
Output: Initialized list
*/
TAC_list* init_tac_list()
{
	return calloc(1, sizeof(TAC_list));
}

/*
init_arg initializes a tagged union argument with a value and type
Input: Argument value and type
Output: Tagged union of both the argument value and it's type
*/
arg_T* init_arg(void* arg, int type)
{
	arg_T* argument = calloc(1, sizeof(arg_T));
	argument->value = arg;
	argument->type = type;

	return argument;
}

/*
traversal_check_arg checks what type should the argument be, a char pointer of a three address code pointer
Input: Node to check
Output: What type should the argument be
*/
int traversal_check_arg(AST* node)
{
	// For variables, numbers and strings, the type is a string, for anything else, it's a three address code for some operation
	return node->type == AST_VARIABLE || node->type == AST_INT || node->type == AST_STRING ? CHAR_P : TAC_P;
}

/*
list_push pushes an instruction to the TAC list
Input: List to push to, instruction to push
Output: None
*/
void list_push(TAC_list* list, TAC* instruction)
{
	if (!list->size)
		list->head = instruction;
	else
		list->last->next = instruction;

	list->last = instruction;    

	list->size++;
}

/*
traversal_visit is the main function of the tree traversal which returns the optimized IR from the tree
Input: Node to start visiting from (root node)
Output: List of three address codes (IR)
*/
TAC_list* traversal_visit(AST* node)
{
	TAC_list* list = init_tac_list();

	traversal_build_instruction(node, list);

	if (list->head)
		traversal_optimize(list);
	
	return list;
}

/*
traversal_build_instruction checks for the node type and builds a fitting TAC argument
Input: Node to build an instruction from, list to push the instruction to
Output: Instruction
*/
void* traversal_build_instruction(AST* node, TAC_list* list)
{
	void* instruction = NULL;
	unsigned int i = 0;

	// If node is NULL we can exit this function
	if (!node)
		return;
	
	// Check for binary operation
	if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV ||
		node->type == AST_COMPARE)
	{
		return traversal_binop(node, list);
	}
	
	// Match types and use fitting functions
	switch (node->type)
	{

	case AST_PROGRAM: 
		for (i = 0; i < node->size; i++)	// Loop through global variables
		{
			if (node->children[i]->type == AST_VARIABLE_DEC)
				instruction = traversal_build_instruction(node->children[i], list);
		}
					
		for (i = 0; i < node->functionsSize; i++)		// Loop through functions
			instruction = traversal_build_instruction(node->function_list[i], list);
		break;
	case AST_FUNCTION: instruction = traversal_func_dec(node, list); break;
	case AST_COMPOUND: traversal_statements(node, list); break;
	case AST_ASSIGNMENT: instruction = traversal_assignment(node, list); break;
	case AST_VARIABLE_DEC: instruction = traversal_var_dec(node, list); break;	// Variable declerations will become normal assignments
	case AST_FUNC_CALL: instruction = traversal_function_call(node, list); break;
	case AST_INT: instruction = (char*)node->int_value; break;
	case AST_STRING: instruction = (char*)node->name; break;
	case AST_VARIABLE: instruction = (char*)node->name; break;
	case AST_IF: traversal_if(node, list); break;
	case AST_WHILE: traversal_while(node, list); break;
	case AST_RETURN: traversal_return(node, list); break;
					
	}
	
	return instruction;
}

/*
traversal_func_dec builds a TAC instruction for a function declaration
Input: Function declaration node, List to push instruction to
Output: TAC instruction that was made
*/
TAC* traversal_func_dec(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));
	TAC* defAmount = calloc(1, sizeof(TAC));
	TAC* endFunc = calloc(1, sizeof(TAC));
	char* value = NULL;

	// In this triple, arg1 will be the function name and arg2 will be the function return type
	instruction->arg1 = init_arg(node->name, CHAR_P);
	instruction->arg2 = init_arg(typeToString(node->var_type), CHAR_P);

	instruction->op = node->type;

	list_push(list, instruction);

	// We also want to push an TAC instruction that tells us how many parameters are there for the function
	defAmount->op = AST_DEF_AMOUNT;
	value = _itoa(node->size, (char*)calloc(1, numOfDigits(node->size) + 1), 10);
	defAmount->arg1 = init_arg(value, CHAR_P);

	list_push(list, defAmount);

	traversal_statements(node->function_body, list);

	// Each function ends with a func end TAC instruction so we can generate code more efficiently
	endFunc->op = TOKEN_FUNC_END;
	list_push(list, endFunc);

	return instruction;
}

/*
traversal_var_dec builds a TAC instruction for variable declarations
Input: Variable declaration node, list to push to
Output: TAC instruction that was made
*/
TAC* traversal_var_dec(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));
	char* buffer = NULL;

	instruction->op = AST_VARIABLE_DEC;
	instruction->arg1 = init_arg(node->name, CHAR_P);

	list_push(list, instruction);

	// All integers will be 4 bytes long
	if (node->var_type == DATA_INT)
	{
		instruction->arg2 = init_arg(dataToAsm(node->var_type), CHAR_P);
	}	
	// For strings however, we want to know the size of the string so we can later create a fitting size in the memory
	else if (node->var_type == DATA_STRING)
	{
		if (!node->value)
		{
			printf("[Error]: String must be initialized with a value");
			exit(1);
		}

		buffer = calloc(1, numOfDigits(strlen(node->value->rightChild->name)) + 1);
		sprintf(buffer, "%d", strlen(node->value->rightChild->name) + 1);
		instruction->arg2 = init_arg(buffer, CHAR_P);
	}

	traversal_build_instruction(node->value, list);

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

	instruction->op = node->type == AST_COMPARE ? node->type_c : node->type;

	instruction->arg1 = init_arg(traversal_build_instruction(node->leftChild, list), traversal_check_arg(node->leftChild));
	instruction->arg2 = init_arg(traversal_build_instruction(node->rightChild, list), traversal_check_arg(node->rightChild));

	list_push(list, instruction);

	return instruction;
}

/*
traversal_assignment builds a TAC instruction for variable assignments
Input: Assignment node, list to push to
Output: TAC instruction that was made
*/
TAC* traversal_assignment(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = node->type;
	instruction->arg1 = init_arg(node->leftChild->name, CHAR_P);

	instruction->arg2 = init_arg(traversal_build_instruction(node->rightChild, list), traversal_check_arg(node->rightChild));

	list_push(list, instruction);

	return instruction;
}

/*
traversal_function_call builds a TAC instruction for function calls
Input: Function call node, list to push to
*/
TAC* traversal_function_call(AST* node, TAC_list* list)
{
	TAC* instruction = NULL;
	TAC* param = NULL;
	int i = 0;
	size_t counter = 0;
	size_t size = 0;
	
	instruction = calloc(1, sizeof(TAC));

	instruction->arg1 = init_arg(node->name, CHAR_P);

	// Check if function is built in
	instruction->op = !strcmp(node->name, "print") ? AST_PRINT : node->type;

	// Arg2 will be the number of arguments passing into the function
	instruction->arg2 = init_arg(calloc(1, numOfDigits(node->size) + 1), CHAR_P);
	_itoa(node->size, instruction->arg2->value, 10);

	// The second argument of the instruction is going to be the amount of arguments passed to the function
	list_push(list, instruction);

	// Push params for function call
	// If function call is a print, push the variables in order, otherwise, we push them in reverse to the stack 
	// So we can pop them correctly in Assembly
	
	for (i = 0; instruction->op == AST_PRINT && i < node->size; i++)
	{
		param = calloc(1, sizeof(TAC));
		param->arg1 = init_arg(traversal_build_instruction(node->arguments[i], list), traversal_check_arg(node->arguments[i]));
		param->op = AST_PARAM;
		list_push(list, param);
	}

	for (i = node->size - 1; instruction->op != AST_PRINT && i >= 0; i--)
	{
		param = calloc(1, sizeof(TAC));
		param->arg1 = init_arg(traversal_build_instruction(node->arguments[i], list), traversal_check_arg(node->arguments[i]));
		param->op = AST_PARAM;
		list_push(list, param);
	}
	
	
	
	return instruction;

}

/*
traversal_condition builds a TAC instruction for a condition
Input: Condition node, list to push instruction to
Output: Instruction that was made
*/
TAC* traversal_condition(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	// For conditions we actually want to check if they are false rather than true to later generate
	// a jump Assembly instruction if condition was not met
	instruction->op = AST_IFZ;			// If zero (If false)

	// If there's no relation operation (<, >, <= etc)
	instruction->arg1 = node->type_c == TOKEN_NOOP ? init_arg(traversal_build_instruction(node->value, list), traversal_check_arg(node->value)) : init_arg(traversal_binop(node, list), TAC_P);
		
	list_push(list, instruction);

	return instruction;
}

/*
traversal_if builds a TAC instruction for an if statement
Input: If node, list to push instruction to
Output: None
*/
void traversal_if(AST* node, TAC_list* list)
{
	TAC* instruction = traversal_condition(node->condition, list);		// First push the condition of the statement

	TAC* label1 = NULL;
	TAC* label2 = calloc(1, sizeof(TAC));
	TAC* gotoInstruction = NULL;

	unsigned int i = 0;

	traversal_statements(node->if_body, list);	// Then traversal through all the statements within the if block
	
	label2->op = AST_LABEL;			// Generate a label to jump to for after the if and optional else statements

	if (node->else_body)
	{
		gotoInstruction = calloc(1, sizeof(TAC));
		label1 = calloc(1, sizeof(TAC));
		label1->op = AST_LABEL;

		gotoInstruction->op = AST_GOTO;
		list_push(list, gotoInstruction);

		list_push(list, label1);	// Label to jump to if the if is false and there's an else statement

		// Assign the goto of if to the start of the else
		instruction->arg2 = init_arg(label1, TAC_P);

		traversal_statements(node->else_body, list);	// Traversal through all the statements within the else block

		list_push(list, label2);
		gotoInstruction->arg1 = init_arg(label2, TAC_P);

	}
	else
	{
		// Label if there's no else, to jump to if the if is false
		list_push(list, label2);
		instruction->arg2 = init_arg(label2, TAC_P);
	}	
}

/*
traversal_while builds a TAC instruction for a while node
Input: While node, list to push instruction to
Output: None
*/
void traversal_while(AST* node, TAC_list* list)
{
	TAC* condition = NULL;
	TAC* gotoInstruction = calloc(1, sizeof(TAC));
	TAC* label1 = calloc(1, sizeof(TAC));
	TAC* label2 = calloc(1, sizeof(TAC));

	label1->op = AST_LOOP_LABEL;
	list_push(list, label1);

	condition = traversal_condition(node->condition, list);		// Create a condition
	
	traversal_statements(node->if_body, list);		// Travel through the body of the while loop and create instructions

	// Jump to the start of the loop including the condition
	gotoInstruction->op = AST_GOTO;
	gotoInstruction->arg1 = init_arg(label1, TAC_P);	
	list_push(list, gotoInstruction);

	label2->op = AST_LABEL;
	list_push(list, label2);

	condition->arg2 = init_arg(label2, TAC_P);

}

/*
traversal_return builds a TAC instruction for return statements
Input: Return node, list to push instruction to
Output: Instruction that was made
*/
TAC* traversal_return(AST* node, TAC_list* list)
{
	TAC* instruction = calloc(1, sizeof(TAC));

	instruction->op = AST_RETURN;
	instruction->arg1 = init_arg(traversal_build_instruction(node->value, list), traversal_check_arg(node->value));

	list_push(list, instruction);

	return instruction;
}

/*
traversal_statements builds TAC instructions for a whole block
Input: Compound node, list to push instructions to
Output: None
*/
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

/*
traversal_print_instructions prints the list of instructions one by one
Input: List of instructions to print
Output: None
*/
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
			// Check if arg1 is a string or a pointer to another TAC
			triple->arg1->type == CHAR_P ? printf("Arg1: %s, ", triple->arg1->value) : printf("Arg1: %p, ", triple->arg1->value);	
		}

		if (triple->arg2)
		{
			// Check if arg2 is a string or a pointer to another TAC
			triple->arg2->type == CHAR_P ? printf("Arg2: %s, ", triple->arg2->value) : printf("Arg2: %p, ", triple->arg2->value);
				
		}

		printf("Address: %p\n", triple);

		triple = triple->next;
	}
}

/*
dataToAsm takes a data type and returns the way we want to print it in the Assembly file
Input: Data type
Output: Assembly representation
*/
char* dataToAsm(int type)
{
	switch (type)
	{
	case DATA_INT: return "DWORD";
	//case DATA_STRING: return "BYTE";
	}

}

/*
traversal_optimize optimizes the TAC list that was made by the traversal
Input: TAC instruction list
Output: None
*/
void traversal_optimize(TAC_list* list)
{
	TAC* instruction = list->head;
	TAC* next = NULL;
	TAC* label = NULL;


	while (instruction)
	{
		// Sometimes we may create labels that jump to other labels, in that case we want the first label to instantly
		// jump to the correct table

		if (instruction->op == AST_IFZ)
		{
			label = instruction->arg2->value;
			next = label->next;

			while (next->op == TOKEN_LBRACE || next->op == TOKEN_RBRACE)
			{
				next = next->next;
			}
				
			if (next->op == AST_GOTO)
			{
				instruction->arg2->value = next->arg1->value;
				traversal_remove_triple(list, label);
			}
				

		}
		// This is the same for ifz but this time for GOTO's that go to a label that jumps to another label
		else if (instruction->op == AST_GOTO)
		{
			label = instruction->arg1->value;
			next = label->next;

			while (next->op == TOKEN_LBRACE || next->op == TOKEN_RBRACE)
			{
				next = next->next;
			}

			if (next->op == AST_GOTO)
			{
				instruction->arg1->value = next->arg1->value;
				traversal_remove_triple(list, label);
			}

		}

		instruction = instruction->next;
	}
}

/*
traversal_remove_triple removes a TAC instruction from the list of instructions
Input: List, triple to remove
Output: None
*/
void traversal_remove_triple(TAC_list* list, TAC* triple)
{
	TAC* instruction = list->head;

	while (instruction)
	{
		if (instruction->next == triple)
		{
			// Removing triple
			instruction->next = triple->next;
			list->size--;
			free(triple);
		}

		instruction = instruction->next;
	}
}

/*
traversal_free_array frees the entire TAC list
Input: TAC list
Output: None
*/
void traversal_free_array(TAC_list* list)
{
	TAC* triple = list->head;
	TAC* prev = NULL;

	while (triple->next)
	{
		// Some special cases where we allocate a new string value rather than use already made one
		if (triple->op == AST_FUNC_CALL || triple->op == AST_PRINT)
		{
			free(triple->arg2->value);
		}
		if (triple->op == AST_DEF_AMOUNT)
		{
			free(triple->arg1->value);
		}
		if (triple->op == AST_VARIABLE_DEC && isNum(triple->arg2->value))
		{
			free(triple->arg2->value);
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
 
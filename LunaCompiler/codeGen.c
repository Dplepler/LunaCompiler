#include "codeGen.h"

register_list* init_registers(table_T* table, TAC* head)
{
	register_list* registerList = calloc(1, sizeof(register_list));
	registerList->registers = calloc(1, sizeof(register_T*));
	unsigned int i = 0;
	
	for (i = 0; i < REG_AMOUNT; i++)
	{
		registerList->registers[i] = calloc(1, sizeof(register_T));
		registerList->registers[i]->reg = i;		// Assigning each register it's name
	}  
	  
	registerList->table = table;
	registerList->instruction = head;

	return registerList;
}

void descriptor_push(register_T* reg, arg_T* descriptor)
{
	reg->regDescList = realloc(reg->regDescList, sizeof(arg_T*) * ++reg->size);
	reg->regDescList[reg->size - 1] = descriptor;
}

void write_asm(table_T* table, TAC* head)
{
	register_list* registerList = init_registers(table, head);
	size_t tableIndex = 0;

	while (registerList->instruction)
	{
		// If we reached the start of a new block, go to the fitting symbol table for that block
		if (registerList->instruction->op == TOKEN_LBRACE)
		{
			registerList->table = registerList->table->nestedScopes[tableIndex];
			tableIndex++;
		}
		// If we're done with a block, reset the index of the table and go to the previous one
		else if (registerList->instruction->op == TOKEN_RBRACE)
		{
			registerList->table = registerList->table->prev;
			tableIndex = 0;
		}
		else
		{
			generate_asm(registerList);
		}

		registerList->instruction = registerList->instruction->next;
	}
}

char* generate_asm(register_list* registerList)
{
	register_T* reg = NULL;

	const char* blankTemplate = "%s %s, %s";
	
	char* op = NULL;
	char* arg1 = NULL;
	char* arg2 = NULL;
	char* asm = NULL;

	op = typeToString(registerList->instruction->op);

	if (registerList->instruction->op == AST_ADD || registerList->instruction->op == AST_SUB || registerList->instruction->op == AST_MUL || registerList->instruction->op == AST_DIV)
	{
		reg = generate_get_register(registerList, registerList->instruction->arg1);
		arg1 = generate_assign_reg(reg, registerList->instruction->arg1->value);

	
		printf("ASM: MOV %s, %s\n", generate_get_register_name(reg), registerList->instruction->arg1->value);
		
		


		reg = generate_get_register(registerList, registerList->instruction->arg2);
		arg2 = generate_assign_reg(reg, registerList->instruction->arg2->value);



		asm = calloc(1, strlen(arg1) + strlen(arg2) + strlen(op) + ASM_INSTRUCTION_SIZE + 1);	// Size of the entire line of code
		sprintf(asm, blankTemplate, op, arg1, arg2);
	}
	else if (registerList->instruction->op == AST_ASSIGNMENT)
	{
		reg = generate_get_register(registerList, registerList->instruction->arg2);
		descriptor_push(reg, registerList->instruction->arg1);
	}
	
	return asm;
}

/*
generate_assign_reg returns a valid register / const number string depending on the given register
Input: A register, an argument to return if register is null
Output: A string that can either represent the given register or the given value, depending on if the register is null or not
*/
char* generate_assign_reg(register_T* r, void* argument)
{
	char* arg = NULL;
	
	if (r)
	{
		arg = generate_get_register_name(r);
	}
	else
	{
		arg = argument;
	}

	return arg;
}

/*
generate_check_variable_in_reg checks if a variable exists in any general purpose register, and if so it returns the register
Input: Register list, variable to search
Output: First register to contain the variable, NULL if none of them do
*/
register_T* generate_check_variable_in_reg(register_list* registerList, void* var)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		for (i2 = 0; i2 < registerList->registers[i]->size; i2++)
		{
			//printf("Register [%s] = %s, var = %s\n", generate_get_register_name(registerList->registers[i]), registerList->registers[i]->regDescList[i2]->value, var);
			if (registerList->registers[i]->regDescList[i2]->value == var)
			{
				reg = registerList->registers[i];
				break;
			}
		}
	}

	return reg;
}

register_T* generate_find_free_reg(register_list* registerList)
{
	unsigned int i = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (!registerList->registers[i]->size)
			reg = registerList->registers[i];

	}

	return reg;
}

register_T* generate_get_register(register_list* registerList, arg_T* arg)
{
	entry_T* entry = NULL;
	register_T* reg = NULL;	
	
	reg = generate_check_variable_in_reg(registerList, arg->value);	// Check if variable is in a register
	if (!reg)
	{
		reg = generate_find_free_reg(registerList);

		if (!reg)
		{
			reg = generate_find_used_reg(registerList);		// Check if there's a register that can be used despite being occupied
		}

		// Push the new variable descriptor onto the register
		descriptor_push(reg, arg);
	}

	return reg;
}

/*
generate_find_lowest_values finds and returns the register with the lowest amount of values it stores
Input: Register list
Output: Register with lowest variables
*/
register_T* generate_find_lowest_values(register_list* registerList)
{
	unsigned int i = 0;
	size_t loc = 0;

	for (i = 1; i < GENERAL_REG_AMOUNT; i++)
	{
		if (registerList->registers[i]->size < registerList->registers[loc]->size)
		{
			loc = i;
		}
	}

	return registerList->registers[loc];
}

register_T* generate_find_used_reg(register_list* registerList)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	// Going through all the variables in all the registers and searching to see if there's a register
	// that has all it's values stores somewhere else as well, if so, we can use that register
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		reg = registerList->registers[i];

		for (i2 = 0; i2 < registerList->registers[i]->size && reg; i2++)
		{
			// If there's a temporary in the register, we cannot use the register
			if (registerList->registers[i]->regDescList[i2]->type == TAC_P)
			{
				reg = NULL;
			}
			else if (table_search_entry(registerList->table, registerList->registers[i]->regDescList[i2]->value)->size <= 1)
			{
				reg = NULL;
			}
		}
	}
	// Check that the variable the program is assigning to doesn't equal both operands
	// and if it doesn't, we can use a register that contains the variable
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (registerList->instruction->next && registerList->instruction->next->op == AST_ASSIGNMENT)
		{
			reg = generate_check_useless_value(registerList, registerList->registers[i]);
		}
	}
	
	// Going through registers and checking if there's a register that holds values that won't be used again
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		reg = generate_check_variable_usabilty(registerList, registerList->registers[i]);	
	}
	// If there are no usable registers, we need to spill the values of one of the registers
	if (!reg)
	{
		reg = generate_find_lowest_values(registerList);
		generate_spill(registerList, reg);
	}

	// Now that the variable is in a new register, we need to free the previous values stored in it and reset the size
	if (reg->regDescList)
	{
		free(reg->regDescList);
		reg->regDescList = NULL;
		reg->size = 0;
	}
	

	return reg;
}

register_T* generate_check_useless_value(register_list* registerList, register_T* r)
{
	register_T* reg = NULL;
	unsigned int i = 0;

	reg = r;

	for (i = 0; i < r->size && reg; i++)
	{
		if (registerList->instruction->next->arg1->value == registerList->instruction->arg1->value
			|| registerList->instruction->next->arg1->value == registerList->instruction->arg2->value
			|| registerList->instruction->next->arg1->value != r->regDescList[i]->value)
		{
			reg = NULL;
		}
	}

	return reg;
}


register_T* generate_check_variable_usabilty(register_list* registerList, register_T* r)
{
	register_T* reg = NULL;
	TAC* triple = registerList->instruction->next;
	unsigned int i = 0;

	reg = r;

	for (i = 0; i < r->size && reg; i++)
	{
		// Loop through instructions until the end of the block to see if we can use a register that holds
		// a value that will not be used
		while (triple->op != TOKEN_RBRACE && reg)
		{
			if (triple->op != AST_ASSIGNMENT)
			{
				// If arg1 or arg2 equals the variable then it will be used
				if (triple->arg1)
					if (triple->arg1->value == r->regDescList[i]->value)
						reg = NULL;
				if (triple->arg2)
					if (triple->arg2->value == r->regDescList[i]->value)
						reg = NULL;
				
			}

			triple = triple->next;
		}
	}

	return reg;
}

void generate_spill(register_list* registerList, register_T* r)
{
	const char* spillTemplate = "MOV [%s], %s"; 
	char* asm = NULL;
	unsigned int i = 0;

	// For each value, allocate a buffer the size of a MOV instruction + register size (2) + variable size
	// and store the value of the variable in itself
	for (i = 0; i < r->size; i++)
	{
		asm = calloc(1, strlen(r->regDescList[i]->value) + CHAR_SIZE_OF_REG + strlen(spillTemplate) - 3);
		sprintf(asm, spillTemplate, r->regDescList[i]->value, generate_get_register_name(r));

		push_address(table_search_entry(registerList->table, r->regDescList[i]->value), r->regDescList[i]->value);
	}
}


char* generate_get_register_name(register_T* r)
{
	char* name = NULL;			

	// Switch types
	switch (r->reg)
	{
		case REG_AX: return "AX";
		case REG_BX: return "BX";
		case REG_CX: return "CX";
		case REG_DX: return "DX";
		case REG_CS: return "CS";
		case REG_DS: return "DS";
		case REG_SS: return "SS";
		case REG_SP: return "SP";
		case REG_SI: return "SI";
		case REG_DI: return "DI";
		case REG_BP: return "BP";
	}
}


void free_registers(register_list* registerList)
{
	unsigned int i = 0;

	for (i = 0; i < REG_AMOUNT; i++)
	{
		free(registerList->registers[i]);
		free(registerList->registers[i]->regDescList);
	}
		

	free(registerList->registers);
	free(registerList);
}
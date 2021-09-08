#include "codeGen.h"

register_list* init_registers(table_T* table, TAC* head)
{
	register_list* registerList = calloc(1, sizeof(register_list));
	registerList->registers = calloc(1, sizeof(register_T*));
	unsigned int i = 0;
	
	for (i = 0; i < REG_AMOUNT; i++)
	{
		registerList->registers[i] = calloc(1, sizeof(register_T));
		//registers_list[i]->regDesc = calloc(1, sizeof(char*));
		registerList->registers[i]->reg = i;		// Assigning each register it's name
	}  
	  
	registerList->table = table;
	registerList->instruction = head;

	return registerList;
}

void push_descriptor(register_T* reg, char* descriptor)
{
	reg->regDesc = realloc(reg->regDesc, sizeof(char*) * ++reg->size);
	reg->regDesc[reg->size - 1] = descriptor;
}

void write_asm(table_T* table, TAC* head)
{
	register_list* registerList = init_registers(table, head);
	while (registerList->instruction)
	{
		generate_asm(registerList);
		registerList->instruction = registerList->instruction->next;
	}
}

char* generate_asm(register_list* registerList)
{
	const char* binopTemplate = "%s %s, %s";
	char* arg1 = NULL;
	char* arg2 = NULL;
	char* result = NULL;

	char* asm = NULL;

	if (registerList->instruction->op == AST_ADD || registerList->instruction->op == AST_SUB || registerList->instruction->op == AST_MUL || registerList->instruction->op == AST_DIV)
	{
		arg1 = generate_get_register(registerList, registerList->instruction->arg1);


		asm = calloc(1, strlen(arg1) + strlen(arg2) + 4);	// Size will equal both arguments 
	}
}



/*
generate_check_variable_in_reg checks if a variable exists in any general purpose register, and if so it returns the register
Input: Register list, variable to search
Output: First register to contain the variable, NULL if none of them do
*/
register_T* generate_check_variable_in_reg(register_list* registerList, char* var)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		for (i2 = 0; i2 < registerList->registers[i]->size && !reg; i2++)
		{
			if (!strcmp(registerList->registers[i]->regDesc[i2], var))
			{
				reg = registerList->registers[i];
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

char* generate_get_register(register_list* registerList, void* arg)
{
	entry_T* entry = NULL;
	register_T* reg = NULL;
	char* val = NULL;

	if (table_search_entry(registerList->table, arg)) // Check to see if arg is a variable
	{
		reg = generate_check_variable_in_reg(registerList, arg);	// Check if variable is in a register
		if (!reg)
		{
			reg = generate_find_free_reg(registerList);
		}
		if (!reg)
		{
			reg = generate_find_used_reg(registerList);		// Check if there's a register that can be used despite being occupied
		}

		val = generate_get_register_name(reg);
	}	
	// If value is not a variable, it is a constant number
	else
	{
		val = arg;
	}


	return val;
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
			if (table_search_entry(registerList->table, registerList->registers[i]->regDesc[i2])->size <= 1)
			{
				reg = NULL;
			}
		}
	}
	// Check that the variable the program is assigning to doesn't equal both operands
	// And if it doesn't, we can use a register that contains the variable
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (registerList->instruction->next && registerList->instruction->next->op == AST_ASSIGNMENT)
		{
			if (strcmp(registerList->instruction->next->arg1, registerList->instruction->arg1)
				&& strcmp(registerList->instruction->next->arg1, registerList->instruction->arg2)
				&& registerList->registers[i]->size == 1)
			{
				reg = registerList->registers[i];
			}
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
		while (triple && reg)
		{
			if (triple->op != AST_ASSIGNMENT && (!strcmp(triple->arg1, r->regDesc[i]) || !strcmp(triple->arg2, r->regDesc[i])))
			{
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
		asm = calloc(1, strlen(r->regDesc[i]) + CHAR_SIZE_OF_REG + strlen(spillTemplate) - 3);
		sprintf(asm, spillTemplate, r->regDesc[i], generate_get_register_name(r));
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
		free(registerList->registers[i]);

	free(registerList->registers);
	free(registerList);
}
#include "codeGen.h"

asm_backend* init_asm_backend(table_T* table, TAC* head, char* targetName)
{
	asm_backend* backend = calloc(1, sizeof(asm_backend));
	backend->registers = calloc(REG_AMOUNT, sizeof(register_T*));
	unsigned int i = 0;
	
	for (i = 0; i < REG_AMOUNT; i++)
	{
		backend->registers[i] = calloc(1, sizeof(register_T));
		backend->registers[i]->reg = i;		// Assigning each register it's name
	}  
	  
	backend->table = table;
	backend->instruction = head;

	backend->targetProg = fopen(targetName, "w");

	return backend;
}

void descriptor_push(register_T* reg, arg_T* descriptor)
{
	reg->regDescList = realloc(reg->regDescList, sizeof(arg_T*) * ++reg->size);
	reg->regDescList[reg->size - 1] = descriptor;
}

/*
descriptor_push_tac pushes a temporary into the register
Input: Desired register, instruction to push
Output: None
*/
void descriptor_push_tac(register_T* reg, TAC* instruction)
{
	descriptor_reset(reg);		
	descriptor_push(reg, init_arg(instruction, TAC_P));
}

void descriptor_reset(register_T* r)
{
	if (r->regDescList)
	{
		free(r->regDescList);
		r->regDescList = NULL;
		r->size = 0;
	}
}

void write_asm(table_T* table, TAC* head, char* targetName)
{
	asm_backend* backend = init_asm_backend(table, head, targetName);
	
	while (backend->instruction)
	{
		// If we reached the start of a new block, go to the fitting symbol table for that block
		if (backend->instruction->op == TOKEN_LBRACE)
		{
			printf("----------------\n");
			backend->table->tableIndex++;
			backend->table = backend->table->nestedScopes[backend->table->tableIndex - 1];
		}
		// If we're done with a block, reset the index of the table and go to the previous one
		else if (backend->instruction->op == TOKEN_RBRACE)
		{
			backend->table = backend->table->prev;
		}
		else
		{
			generate_asm(backend);
		}

		backend->instruction = backend->instruction->next;
	}

	fclose(backend->targetProg);
	free_registers(backend);
}

void generate_asm(asm_backend* backend)
{
	register_T* reg1 = NULL;
	register_T* reg2 = NULL;

	const char* blankTemplate = "%s %s, %s";
	
	char* op = NULL;
	char* arg1 = NULL;
	char* arg2 = NULL;
	entry_T* entry1 = NULL;
	entry_T* entry2 = NULL;

	if (backend->instruction->arg1)
		entry1 = table_search_entry(backend->table, backend->instruction->arg1->value);

	if (backend->instruction->arg2)
		entry2 = table_search_entry(backend->table, backend->instruction->arg2->value);

	op = typeToString(backend->instruction->op);

	if (backend->instruction->op == AST_ADD || backend->instruction->op == AST_SUB || backend->instruction->op == AST_MUL || backend->instruction->op == AST_DIV)
	{
		reg1 = generate_get_register(backend, backend->instruction->arg1);
		arg1 = generate_assign_reg(reg1, backend->instruction->arg1->value);

		// If the value in the second argument is a variable or temp, we want to use a register
		if (entry2 || backend->instruction->arg2->type == TAC_P)
		{
			reg2 = generate_get_register(backend, backend->instruction->arg2);
			arg2 = generate_assign_reg(reg2, backend->instruction->arg2->value);
		}
		// If the value of the second argument is a number, we can treat it as a const instead of putting it
		// in a new register
		else
		{
			arg2 = backend->instruction->arg2->value;
		}
		
		descriptor_push_tac(reg1, backend->instruction);			// We treat the whole TAC as a temporary variable that is now in the register

		//asm = calloc(1, strlen(arg1) + strlen(arg2) + strlen(op) + ASM_INSTRUCTION_SIZE + 1);	// Size of the entire line of code
		fprintf(backend->targetProg, "[ASM]: %s %s, %s\n", op, arg1, arg2);


	}
	else if (backend->instruction->op == AST_ASSIGNMENT)
	{
		reg1 = generate_get_register(backend, backend->instruction->arg2);
		address_reset(entry1);
		address_push(entry1, reg1);
		descriptor_push(reg1, backend->instruction->arg1);
	}

	else if (backend->instruction->op == AST_FUNCTION)
	{
		fprintf(backend->targetProg, "[ASM]: %s PROC ", backend->instruction->arg1);	// Generating function label
		
	}
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
register_T* generate_check_variable_in_reg(asm_backend* backend, void* var)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		for (i2 = 0; i2 < backend->registers[i]->size; i2++)
		{
			if (backend->registers[i]->regDescList[i2]->value == var 
				|| !strcmp(backend->registers[i]->regDescList[i2]->value, var))
			{
				reg = backend->registers[i];
				break;
			}
		}
	}

	return reg;
}

register_T* generate_find_free_reg(asm_backend* backend)
{
	unsigned int i = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (!backend->registers[i]->size)
			reg = backend->registers[i];
	}

	return reg;
}

register_T* generate_get_register(asm_backend* backend, arg_T* arg)
{
	entry_T* entry = NULL;
	register_T* reg = NULL;	
	
	reg = generate_check_variable_in_reg(backend, arg->value);	// Check if variable is in a register
	if (!reg)
	{
		reg = generate_find_free_reg(backend);

		if (!reg)
		{
			reg = generate_find_used_reg(backend);		// Check if there's a register that can be used despite being occupied
		}

		// Push the new variable descriptor onto the register
		descriptor_push(reg, arg);

		if (entry = (table_search_entry(backend->table, arg->value)))
			address_push(entry, reg);

		printf("[ASM]: MOV %s, %s\n", generate_get_register_name(reg), arg->value);
	}

	return reg;
}

/*
generate_find_lowest_values finds and returns the register with the lowest amount of values it stores
Input: Register list
Output: Register with lowest variables
*/
register_T* generate_find_lowest_values(asm_backend* backend)
{
	unsigned int i = 0;
	size_t loc = 0;

	for (i = 1; i < GENERAL_REG_AMOUNT; i++)
	{
		if (backend->registers[i]->size < backend->registers[loc]->size)
		{
			loc = i;
		}
	}

	return backend->registers[loc];
}

register_T* generate_find_used_reg(asm_backend* backend)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;
	entry_T* entry = NULL;

	// Going through all the variables in all the registers and searching to see if there's a register
	// that has all it's values stored somewhere else as well, if so, we can use that register
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		reg = backend->registers[i];

		for (i2 = 0; i2 < backend->registers[i]->size && reg; i2++)
		{
			// If there's a temporary in the register, we cannot use the register
			if (backend->registers[i]->regDescList[i2]->type == TAC_P)
			{
				reg = NULL;
			}
			else if (entry = table_search_entry(backend->table, backend->registers[i]->regDescList[i2]->value))
			{
				if (entry->size <= 1)
					reg = NULL;
			}
		}
	}
	// Check that the variable the program is assigning to doesn't equal both operands
	// and if it doesn't, we can use a register that contains the variable
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (backend->instruction->next && backend->instruction->next->op == AST_ASSIGNMENT)
		{
			reg = generate_check_useless_value(backend, backend->registers[i]);
		}
	}
	
	// Going through registers and checking if there's a register that holds values that won't be used again
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		reg = generate_check_variable_usabilty(backend, backend->registers[i]);	
	}
	// If there are no usable registers, we need to spill the values of one of the registers
	if (!reg)
	{
		reg = generate_find_lowest_values(backend);
		generate_spill(backend, reg);
	}

	// Now that the variable is in a new register, we need to free the previous values stored in it and reset the size
	if (reg->regDescList)
	{
		descriptor_reset(reg);
	}
	

	return reg;
}

register_T* generate_check_useless_value(asm_backend* backend, register_T* r)
{
	register_T* reg = NULL;
	unsigned int i = 0;

	reg = r;

	for (i = 0; i < r->size && reg; i++)
	{
		if (backend->instruction->next->arg1->value == backend->instruction->arg1->value
			|| backend->instruction->next->arg1->value == backend->instruction->arg2->value
			|| backend->instruction->next->arg1->value != r->regDescList[i]->value)
		{
			reg = NULL;
		}
	}

	return reg;
}


register_T* generate_check_variable_usabilty(asm_backend* backend, register_T* r)
{
	register_T* reg = NULL;
	TAC* triple = backend->instruction;
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

void generate_spill(asm_backend* backend, register_T* r)
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

		address_push(table_search_entry(backend->table, r->regDescList[i]->value), r->regDescList[i]->value);
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


void free_registers(asm_backend* backend)
{
	unsigned int i = 0;

	for (i = 0; i < REG_AMOUNT; i++)
	{	
		if (backend->registers[i]->regDescList)
			free(backend->registers[i]->regDescList);
		free(backend->registers[i]);
	}
		

	free(backend->registers);
	free(backend);
}
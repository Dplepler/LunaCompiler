#include "codeGen.h"



register_list* init_registers()
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
	  
	return registerList;
}

void push_descriptor(register_T* reg, char* descriptor)
{
	reg->regDesc = realloc(reg->regDesc, sizeof(char*) * ++reg->size);
	reg->regDesc[reg->size - 1] = descriptor;
}

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

void generate_asm(register_list* registerList)
{
	const char* binopTemplate = "%s %s, %s";
	char* arg1 = NULL;
	char* arg2 = NULL;
	char* result = NULL;

	char* asm = NULL;

	if (registerList->instruction->op == AST_ADD || registerList->instruction->op == AST_SUB || registerList->instruction->op == AST_MUL || registerList->instruction->op == AST_DIV)
	{
		arg1 = generate_get_register(registerList, registerList->instruction->arg1);
		arg2 = generate_get_register(registerList, registerList->instruction->arg2);

		asm = calloc(1, strlen(arg1) + strlen(arg2) + 4);	// Size will equal both arguments 
	}
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
			reg = generate_find_used_reg(registerList);
		}
		if (!reg)
		{
			if (registerList->instruction->next)
			{
				if (registerList->instruction->next->op == AST_ASSIGNMENT)
				{

				}
			}
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

register_T* generate_find_used_reg(register_list* registerList)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		for (i2 = 0; i2 < registerList->registers[i]->size && !reg; i2++)
		{
			if (table_search_entry(registerList->table, registerList->registers[i]->regDesc[i2])->size > 1)
			{
				reg = registerList->registers[i];
			}
		}
	}

	return reg;
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
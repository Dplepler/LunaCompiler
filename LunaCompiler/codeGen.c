#include "codeGen.h"
#include "template.h"

asm_backend* init_asm_backend(table_T* table, TAC* head, char* targetName)
{
	asm_backend* backend = calloc(1, sizeof(asm_backend));

	unsigned int i = 0;

	backend->registers = calloc(REG_AMOUNT, sizeof(register_T*));
	backend->labelList = calloc(1, sizeof(label_list));
	
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
	descriptor_push(reg, init_arg(instruction, TEMP_P));
}

void descriptor_reset(register_T* r)
{
	unsigned int i = 0;

	for (i = 0; i < r->size; i++)
	{
		if (r->regDescList[i]->type == TEMP_P)
		{
			free(r->regDescList[i]);
			r->regDescList[i]->value = NULL;
			r->regDescList[i] = NULL;
		}

	}

	if (r->regDescList)
	{
		free(r->regDescList);
		r->regDescList = NULL;
		r->size = 0;
	}
}

void generate_global_vars(asm_backend* backend, TAC* triple)
{
	table_T* table = backend->table;

	while (triple)
	{
		switch (triple->op)
		{

		case TOKEN_LBRACE:
			table->tableIndex++;
			table = table->nestedScopes[table->tableIndex - 1];
			break;

		case TOKEN_RBRACE:
			table->tableIndex = 0;
			table = table->prev;
			break;

		case AST_VARIABLE_DEC:
			if (!(table_search_table(table, triple->arg1->value)->prev))
				fprintf(backend->targetProg, "%s %s 0\n", triple->arg1->value, triple->arg2->value);
			break;

		}

		triple = triple->next;
	}
}


void write_asm(table_T* table, TAC* head, char* targetName)
{
	asm_backend* backend = init_asm_backend(table, head, targetName);
	TAC* triple = head;
	TAC* mainStart = NULL;
	int mainTableIndex = 0;
	
	unsigned int i = 0;

	for (i = 0; i < TEMPLATE_SIZE; i++)
		fprintf(backend->targetProg, "%s\n", asm_template[i]);
	
	fprintf(backend->targetProg, ".data\n");
	
	generate_global_vars(backend, triple);

	fprintf(backend->targetProg, ".code\n");

	backend->table->tableIndex = 0;

	// Main loop to generate code
	while (backend->instruction)
	{
		// Skipping the main function to only generate it at the end
		if (backend->instruction->op == AST_FUNCTION && !strcmp(backend->instruction->arg1->value, "main"))
		{
			mainStart = backend->instruction;
			mainTableIndex = backend->table->tableIndex;

			backend->table->tableIndex++;

			while (backend->instruction->op != TOKEN_FUNC_END)
				backend->instruction = backend->instruction->next;
		}

		generate_asm(backend);
		backend->instruction = backend->instruction->next;
	}

	backend->table->tableIndex = mainTableIndex;
	backend->table = table;
	backend->instruction = mainStart;

	if (mainStart)
	{
		generate_main(backend);
	}
	else
	{
		printf("[ERROR]: No main file to start executing from\n");
		exit(1);
	}
		
	fclose(backend->targetProg);
	free_registers(backend);
}

void generate_asm(asm_backend* backend)
{
	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	
	char* op = NULL;
	char* arg1 = NULL;
	char* arg2 = NULL;

	op = typeToString(backend->instruction->op);

	if (backend->instruction->op == AST_ADD || backend->instruction->op == AST_SUB)
	{
		generate_binop(backend);
	}
	else if (backend->instruction->op == AST_MUL || backend->instruction->op == AST_DIV)
	{
		generate_mul_div(backend);
	}
	else if (backend->instruction->op == TOKEN_LESS || backend->instruction->op == TOKEN_MORE
		|| backend->instruction->op == TOKEN_ELESS || backend->instruction->op == TOKEN_EMORE
		|| backend->instruction->op == TOKEN_DEQUAL || backend->instruction->op == TOKEN_NEQUAL)
	{
		generate_condition(backend);
	}
	else
	{
		switch (backend->instruction->op)
		{
		
		case TOKEN_LBRACE:
			// If we reached the start of a new block, go to the fitting symbol table for that block
			backend->table->tableIndex++;
			backend->table = backend->table->nestedScopes[backend->table->tableIndex - 1];
			break;

		case TOKEN_RBRACE: generate_block_exit(backend); 
			// When done with a block, reset the index of the table and go to the previous one break;
			backend->table->tableIndex = 0;
			backend->table = backend->table->prev;
			break;

		case AST_FUNCTION: generate_function(backend); break;
		case AST_ASSIGNMENT: generate_assignment(backend); break;
		case AST_VARIABLE_DEC: generate_var_dec(backend); break;
		case AST_IFZ: generate_if_false(backend); break;
		case AST_GOTO: generate_unconditional_jump(backend); break;
		case AST_LABEL: fprintf(backend->targetProg, "%s:\n", generate_get_label(backend, backend->instruction)); break;
		case AST_LOOP_LABEL: fprintf(backend->targetProg, "%s:\n", generate_get_label(backend, backend->instruction)); generate_block_exit(backend); break;
		case AST_FUNC_CALL: generate_func_call(backend); break;
		case AST_RETURN: generate_return(backend); break;
	
		
		}
	}
}

void generate_binop(asm_backend* backend)
{
	register_T* reg1 = NULL;
	register_T* reg2 = NULL;

	char* arg1 = NULL;
	char* arg2 = NULL;

	entry_T* entry = table_search_entry(backend->table, backend->instruction->arg1->value);

	if (entry)
		address_reset(entry);		// The variable being added to will not hold previous numbers anymore

	// If the operation is add or sub by 0 then we can do nothing 
	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "0"))
	{
		descriptor_push_tac(generate_move_to_register(backend, backend->instruction->arg1), backend->instruction);
		return;
	}
	if (backend->instruction->op == AST_ADD && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "0"))
	{
		descriptor_push_tac(generate_move_to_register(backend, backend->instruction->arg2), backend->instruction);
		return;
	}


	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "1"))
	{
		reg1 = generate_move_to_register(backend, backend->instruction->arg1);
		descriptor_push_tac(reg1, backend->instruction);
		backend->instruction->op == AST_ADD ? fprintf(backend->targetProg, "INC %s\n", generate_get_register_name(reg1)) : fprintf(backend->targetProg, "DEC %s\n", generate_get_register_name(reg1));
		return;
	}
	if (backend->instruction->op == AST_ADD && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "1"))
	{
		reg1 = generate_move_to_register(backend, backend->instruction->arg2);
		descriptor_push_tac(reg1, backend->instruction);
		fprintf(backend->targetProg, "INC %s\n", generate_get_register_name(reg1));
		return;
	}

	reg1 = generate_move_to_register(backend, backend->instruction->arg1);

	arg1 = generate_assign_reg(reg1, backend->instruction->arg1->value);
	arg2 = NULL;
	
	// If the value in the second argument is a variable or temp, we want to use a register
	if (table_search_entry(backend->table, backend->instruction->arg2->value) || backend->instruction->arg2->type == TAC_P)
	{
		reg1->regLock = true;
		reg2 = generate_move_to_register(backend, backend->instruction->arg2);
		reg1->regLock = false;

		arg2 = generate_assign_reg(reg2, backend->instruction->arg2->value);
	}
	// If the value of the second argument is a number, we can treat it as a const instead of putting it
	// in a new register
	else
	{
		arg2 = backend->instruction->arg2->value;
	}

	descriptor_push_tac(reg1, backend->instruction);			// We treat the whole TAC as a temporary variable that is now in the register

	fprintf(backend->targetProg, "%s %s, %s\n", typeToString(backend->instruction->op), arg1, arg2);
}

void generate_mul_div(asm_backend* backend)
{
	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	entry_T* entry = NULL;

	unsigned int i = 0;

	// If one of the operands is 1 and the operation is multiplication then do nothing as multiplication by 1 means nothing
	// Also if the second argument is 1 and the operation is division we can do nothing as well for the same reason
	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "1"))
	{
		descriptor_push_tac(generate_move_to_register(backend, backend->instruction->arg1), backend->instruction);	
		return;
	}
	if (backend->instruction->op == AST_MUL && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "1"))
	{
		descriptor_push_tac(generate_move_to_register(backend, backend->instruction->arg2), backend->instruction);	
		return;
	}

	reg1 = generate_move_to_ax(backend, backend->instruction->arg1);	
	
	reg1->regLock = true;
	reg2 = generate_move_to_register(backend, backend->instruction->arg2);
	reg1->regLock = false;

	if (backend->instruction->op == AST_MUL)
		fprintf(backend->targetProg, "MUL %s\n", generate_get_register_name(reg2));
	else
		fprintf(backend->targetProg, "DIV %s\n", generate_get_register_name(reg2));

	descriptor_push_tac(backend->registers[REG_AX], backend->instruction);	// Now AX will hold the result value so we can reset it to that value
}

void generate_condition(asm_backend* backend)
{
	fprintf(backend->targetProg, "CMP %s, %s\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)), generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg2)));
	descriptor_push_tac(generate_get_register(backend), backend->instruction);
	
}

void generate_if_false(asm_backend* backend)
{
	char* jmpCondition = NULL;

	if (backend->instruction->arg1->type == TAC_P || backend->instruction->arg1->type == TEMP_P)
	{
		// In all our cases when it comes to comparison, we want to do the exact opposite of what is specified
		switch (((TAC*)backend->instruction->arg1->value)->op)
		{

		case TOKEN_LESS: jmpCondition = "JGE"; break;
		case TOKEN_ELESS: jmpCondition = "JG"; break;
		case TOKEN_MORE: jmpCondition = "JLE"; break;
		case TOKEN_EMORE: jmpCondition = "JL"; break;
		case TOKEN_DEQUAL: jmpCondition = "JNE"; break;
		case TOKEN_NEQUAL: jmpCondition = "JE"; break;

		}

		fprintf(backend->targetProg, "%s %s\n", jmpCondition, generate_get_label(backend, backend->instruction->arg2->value));
	}
	// For a number or variable, we want to skip statement if it equals 0
	else
	{
		fprintf(backend->targetProg, "CMP %s, 0\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)));
		fprintf(backend->targetProg, "JE %s\n", generate_get_label(backend, backend->instruction->arg2->value));
	}
}

void generate_unconditional_jump(asm_backend* backend)
{
	fprintf(backend->targetProg, "JMP %s\n", generate_get_label(backend, backend->instruction->arg1->value));
}

void generate_assignment(asm_backend* backend)
{
	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	entry_T* entry = table_search_entry(backend->table, backend->instruction->arg1->value);

	// If variable equals a function call then the value will return in AX, therefore we know that the register will always be AX
	if (backend->instruction->arg2->type == TAC_P && ((TAC*)backend->instruction->arg2->value)->op == AST_FUNC_CALL)
	{
		reg1 = backend->registers[REG_AX];
	}
	// Otherwise, without a function call, we can use any register
	else
	{
		reg1 = generate_move_to_register(backend, backend->instruction->arg2);
	}

	reg2 = generate_check_variable_in_reg(backend, backend->instruction->arg1);

	address_reset(entry);
	
	address_push(entry, reg1, ADDRESS_REG);
	descriptor_push(reg1, backend->instruction->arg1);
	
	if (reg2)
		generate_remove_descriptor(reg2, backend->instruction->arg1);
	
}

void generate_var_dec(asm_backend* backend)
{
	if (table_search_table(backend->table, backend->instruction->arg1->value)->prev)
	{
		fprintf(backend->targetProg, "LOCAL %s:%s\n", backend->instruction->arg1->value, backend->instruction->arg2->value);
	}
}

void generate_main(asm_backend* backend)
{
	char* name = backend->instruction->arg1->value;

	fprintf(backend->targetProg, "%s:\n", name);

	backend->instruction = backend->instruction->next->next;

	while (backend->instruction->op != TOKEN_FUNC_END)
	{
		generate_asm(backend);
		backend->instruction = backend->instruction->next;
	}
	generate_block_exit(backend);

	fprintf(backend->targetProg, "end %s\n", name);
}


void generate_function(asm_backend* backend)
{
	size_t counter = atoi(backend->instruction->next->arg1->value);
	char* name = backend->instruction->arg1->value;
	unsigned int i = 0;

	fprintf(backend->targetProg, "%s PROC ", name);	// Generating function label

	backend->table->tableIndex++;
	backend->table = backend->table->nestedScopes[backend->table->tableIndex - 1];

	// Skipping number of local vars and start of block
	backend->instruction = backend->instruction->next->next->next;


	if (counter > 0)
		fprintf(backend->targetProg, "%s:%s", backend->table->entries[i]->name, dataToAsm(backend->table->entries[i]->dtype));

	for (i = 1; i < counter; i++)
	{
		fprintf(backend->targetProg, ", %s:%s", backend->table->entries[i]->name, dataToAsm(backend->table->entries[i]->dtype));
	}

	fprintf(backend->targetProg, "\n");

	while (backend->instruction->op != TOKEN_FUNC_END)
	{
		generate_asm(backend);
		backend->instruction = backend->instruction->next;
	}

	fprintf(backend->targetProg, "%s ENDP\n", name);
}

void generate_return(asm_backend* backend)
{
	register_T* reg = generate_move_to_ax(backend, backend->instruction->arg1);

	fprintf(backend->targetProg, "RET\n");
}

void generate_func_call(asm_backend* backend)
{
	unsigned int i = 0;
	size_t size = atoi(backend->instruction->arg2->value);
	char* name = backend->instruction->arg1->value;

	for (i = 0; i < size; i++)
	{
		backend->instruction = backend->instruction->next;

		// For variables and numbers we can just push them as is
		if (backend->instruction->op == AST_PARAM && backend->instruction->arg1->type == CHAR_P)
		{
			fprintf(backend->targetProg, "PUSH %s\n", backend->instruction->arg1->value);
		}
		// For TAC operations we need to allocate a register before pushing
		else if (backend->instruction->op == AST_PARAM && (backend->instruction->arg1->type == TAC_P || backend->instruction->arg1->type == TEMP_P))
		{
			fprintf(backend->targetProg, "PUSH %s\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)));
		}
		// There can be expression operations between parameters, so we need to generate code for them but make the loop longer
		else
		{
			generate_asm(backend);
			i--;
		}
	}

	fprintf(backend->targetProg, "CALL %s\n", name);
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
register_T* generate_check_variable_in_reg(asm_backend* backend, arg_T* var)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	register_T* reg = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		for (i2 = 0; i2 < backend->registers[i]->size; i2++)
		{
			if (generate_compare_arguments(var, backend->registers[i]->regDescList[i2]))
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

/*
generate_find_register finds a register for an argument, if the argument does not exist in any register it loads it in one
Input: Backend struct with instruction, argument to load register for
Output: Fitting register
*/
register_T* generate_find_register(asm_backend* backend, arg_T* arg)
{
	register_T* reg = generate_check_variable_in_reg(backend, arg);	// Check if variable is in a register
	if (!reg)
		reg = generate_get_register(backend);

	return reg;
}


register_T* generate_get_register(asm_backend* backend)
{
	register_T* reg = generate_find_free_reg(backend);

	if (!reg)
		reg = generate_find_used_reg(backend);		// Check if there's a register that can be used despite being occupied
	

	return reg;
}

register_T* generate_move_to_register(asm_backend* backend, arg_T* arg)
{
	register_T* reg = generate_check_variable_in_reg(backend, arg);			// Check if variable is in a register
	entry_T* entry = NULL;
	char* name = NULL;

	if (!reg)
	{
		entry = table_search_entry(backend->table, arg->value);

		reg = generate_get_register(backend);

		name = generate_get_register_name(reg);

		// Push the new variable descriptor onto the register
		descriptor_push(reg, arg);

		if (entry)
		{
			address_push(entry, reg, ADDRESS_REG);
			fprintf(backend->targetProg, "MOV %s, [%s]\n", name, arg->value);
		}
		else
		{
			
			if (!strcmp(arg->value, "0"))
				fprintf(backend->targetProg, "XOR %s %s\n", name, name);
			else
				fprintf(backend->targetProg, "MOV %s, %s\n", name, arg->value);
		}
	}

	return reg;
}

register_T* generate_move_new_value_to_register(asm_backend* backend, arg_T* arg)
{
	register_T* reg = NULL;
	entry_T* entry = NULL;
	char* name = NULL;

	
	entry = table_search_entry(backend->table, arg->value);

	reg = generate_get_register(backend);

	name = generate_get_register_name(reg);

	// Push the new variable descriptor onto the register
	descriptor_push(reg, arg);

	if (entry)
	{
		address_push(entry, reg, ADDRESS_REG);
		fprintf(backend->targetProg, "MOV %s, [%s]\n", name, arg->value);
	}
	else
	{

		if (!strcmp(arg->value, "0"))
			fprintf(backend->targetProg, "XOR %s %s\n", name, name);
		else
			fprintf(backend->targetProg, "MOV %s, %s\n", name, arg->value);
	}
	

	return reg;
}

register_T* generate_move_to_ax(asm_backend* backend, arg_T* arg)
{
	register_T* reg = generate_check_variable_in_reg(backend, arg);
	unsigned int i = 0;

	if (!reg)
	{
		if ((reg = generate_get_register(backend))->reg != REG_AX)
		{
			// Copying descriptors from available register to AX
			for (i = 0; i < backend->registers[REG_AX]->size; i++)
				descriptor_push(reg, backend->registers[REG_AX]->regDescList[i]);

			fprintf(backend->targetProg, "MOV %s, EAX\n", generate_get_register_name(reg));		// Move the value of AX to a different register

			// Reset AX
			backend->registers[REG_AX]->size = 0;
			free(backend->registers[REG_AX]->regDescList);
			backend->registers[REG_AX]->regDescList = NULL;

		}

		reg = generate_move_to_register(backend, arg);
	}
	else if (reg->reg != REG_AX)
	{
		fprintf(backend->targetProg, "MOV EAX, %s\n", generate_get_register_name(reg));
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

	for (i = 1; i < GENERAL_REG_AMOUNT && !backend->registers[i]->regLock; i++)
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
		if (backend->registers[i]->regLock)
			continue;

		reg = backend->registers[i];

		for (i2 = 0; i2 < backend->registers[i]->size && reg ; i2++)
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
		if (backend->instruction->next && backend->instruction->next->op == AST_ASSIGNMENT 
			&& backend->instruction->next->arg1->type != TAC_P && !backend->registers[i]->regLock)
		{
			reg = generate_check_useless_value(backend, backend->registers[i]);
		}
	}
	
	// Going through registers and checking if there's a register that holds values that won't be used again
	for (i = 0; i < GENERAL_REG_AMOUNT && !reg; i++)
	{
		if (!backend->registers[i]->regLock)
			continue;

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
		if (generate_compare_arguments(backend->instruction->next->arg1->value, backend->instruction->arg1->value)
			|| generate_compare_arguments(backend->instruction->next->arg1->value, backend->instruction->arg2->value)
			|| !generate_compare_arguments(backend->instruction->next->arg1->value, r->regDescList[i]->value))
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
				if (triple->arg1 && generate_compare_arguments(triple->arg1, r->regDescList[i]))
					reg = NULL;
				if (triple->arg2 && generate_compare_arguments(triple->arg2, r->regDescList[i]))
					reg = NULL;
				
			}

			triple = triple->next;
		}
	}

	return reg;
}

void generate_spill(asm_backend* backend, register_T* r)
{
	unsigned int i = 0;

	// For each value, store the value of the variable in itself
	for (i = 0; i < r->size; i++)
	{
		fprintf(backend->targetProg, "MOV [%s], %s\n", r->regDescList[i]->value, generate_get_register_name(r));
		address_push(table_search_entry(backend->table, r->regDescList[i]->value), r->regDescList[i]->value, ADDRESS_VAR);
	}
}

void generate_block_exit(asm_backend* backend)
{
	unsigned int i = 0;
	unsigned int i2 = 0;
	entry_T* entry = NULL;

	for (i = 0; i < GENERAL_REG_AMOUNT; i++)
	{
		for (i2 = 0; i2 < backend->registers[i]->size; i2++)
		{
			entry = table_search_entry(backend->table, backend->registers[i]->regDescList[i2]->value);

			if (backend->registers[i]->regDescList[i2]->type == CHAR_P && !table_search_in_specific_table(backend->table, backend->registers[i]->regDescList[i2]->value)
				&& !table_search_address(entry, backend->registers[i]->regDescList[i2]->value) && entry)
			{
				fprintf(backend->targetProg, "MOV [%s], %s\n", backend->registers[i]->regDescList[i2]->value, generate_get_register_name(backend->registers[i]));
				address_push(entry, backend->registers[i]->regDescList[i2]->value, ADDRESS_VAR);
			}
			else if (backend->registers[i]->regDescList[i2]->type == CHAR_P && entry && table_search_in_specific_table(backend->table, backend->registers[i]->regDescList[i2]->value))
			{
				address_reset(entry);
			}
		}

		descriptor_reset(backend->registers[i]);
	}
}



char* generate_get_label(asm_backend* backend, TAC* label)
{
	unsigned int i = 0;
	char* name = NULL;
	
	// Search for label and return it
	for (i = 0; i < backend->labelList->size && !name; i++)
	{
		if (label == backend->labelList->labels[i])
		{
			name = backend->labelList->names[i];
		}
	}
	// If label was not found, create a new one
	if (!name)
	{
		backend->labelList->labels = realloc(backend->labelList->labels, sizeof(TAC*) * ++backend->labelList->size);	// Appending list
		backend->labelList->labels[backend->labelList->size - 1] = label;
		backend->labelList->names = realloc(backend->labelList->names, sizeof(char*) * backend->labelList->size);
		name = name = calloc(1, strlen("label") + numOfDigits(backend->labelList->size) + 1);
		sprintf(name, "label%d", backend->labelList->size);
		backend->labelList->names[backend->labelList->size - 1] = name;
	}

	return name;
}


char* generate_get_register_name(register_T* r)
{
	char* name = NULL;			

	// Switch types
	switch (r->reg)
	{
		case REG_AX: return "EAX";
		case REG_BX: return "EBX";
		case REG_CX: return "ECX";
		case REG_DX: return "EDX";
		case REG_CS: return "CS";
		case REG_DS: return "DS";
		case REG_SS: return "SS";
		case REG_SP: return "ESP";
		case REG_SI: return "ESI";
		case REG_DI: return "EDI";
		case REG_BP: return "EBP";
	}
}

bool generate_compare_arguments(arg_T* arg1, arg_T* arg2)
{
	bool flag = false;

	if ((arg1->type == TAC_P || arg1->type == TEMP_P) && (arg2->type == TAC_P || arg2->type == TEMP_P) && arg1->value == arg2->value)
	{
		flag = true;
	}
	else if (arg1->type == CHAR_P && arg2->type == CHAR_P && !strcmp(arg1->value, arg2->value))
	{
		flag = true;
	}

	return flag;
}

void generate_remove_descriptor(register_T* reg, arg_T* desc)
{
	unsigned int i = 0;
	unsigned int index = 0;

	while (i < reg->size)
	{
		if (!generate_compare_arguments(reg->regDescList[i], desc))
		{
			reg->regDescList[index] = reg->regDescList[index];
			index++;
		}

		i++;
	}

	reg->regDescList = realloc(reg->regDescList, --reg->size * sizeof(arg_T*));
}

void free_registers(asm_backend* backend)
{
	unsigned int i = 0;

	for (i = 0; i < REG_AMOUNT; i++)
	{	
		descriptor_reset(backend->registers[i]);
		free(backend->registers[i]);
	}
	if (backend->labelList)
	{
		for (i = 0; i < backend->labelList->size; i++)
			free(backend->labelList->names[i]);

		free(backend->labelList->names);
		free(backend->labelList->labels);
		free(backend->labelList);
	}
	
	free(backend->registers);
	free(backend);
}

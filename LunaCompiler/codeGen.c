#include "codeGen.h"
#include "template.h"

/*
init_asm_backend initializes the asm backend
Input: Program symbol table, Head of the TAC instructions list, target name for the produced file (.asm)
Output: The asm backend
*/
asm_backend* init_asm_backend(table_T* table, TAC* head, char* targetName) {

	asm_backend* backend = calloc(1, sizeof(asm_backend));

	backend->registers = calloc(REG_AMOUNT, sizeof(register_T*));
	backend->labelList = calloc(1, sizeof(label_list));
	
	// Allocate all registers for the backend
	for (unsigned int i = 0; i < REG_AMOUNT; i++) {

		backend->registers[i] = calloc(1, sizeof(register_T));
		backend->registers[i]->reg = i;		// Assigning each register it's name
	}  
	
	backend->table = table;
	backend->instruction = head;

	backend->targetProg = fopen(targetName, "w");

	return backend;
}

/*
descriptor_push pushes an argument to the register descriptor
Input: Register to push to, argument to push
Output: None
*/
void descriptor_push(register_T* reg, arg_T* descriptor) {

	reg->regDescList = realloc(reg->regDescList, sizeof(arg_T*) * ++reg->size);
	reg->regDescList[reg->size - 1] = descriptor;
}

/*
descriptor_push_tac pushes a temporary into the register
Input: Backend, desired register, instruction to push
Output: None
*/
void descriptor_push_tac(asm_backend* backend, register_T* reg, TAC* instruction) {

	descriptor_reset(backend, reg);		
	descriptor_push(reg, init_arg(instruction, TEMP_P));
}

/*
descriptor_reset resets the register descriptor to not include any arguments
Input: Backend, register to reset
Output: None
*/
void descriptor_reset(asm_backend* backend, register_T* r) {

	entry_T* entry = NULL;

	// Free all arguments
	for (unsigned int i = 0; i < r->size; i++) {

		if (r->regDescList[i]->type == TEMP_P) {

			free(r->regDescList[i]);
			r->regDescList[i]->value = NULL;
			r->regDescList[i] = NULL;
		}
		else if (r->regDescList[i]->type == CHAR_P && (entry = table_search_entry(backend->table, r->regDescList[i]->value))) {

			address_remove_register(entry, r);
		}
	}

	// Free the register descriptor list itself
	if (r->regDescList) {

		free(r->regDescList);
		r->regDescList = NULL;
		r->size = 0;
	}
}

/*
descriptor_reset_all_registers just automatically resets all register descriptors from all general purpose registers
Input: Backend
Output: None
*/
void descriptor_reset_all_registers(asm_backend* backend) {

	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {
		descriptor_reset(backend, backend->registers[i]);
	}	
}

/*
generate_global_vars generates all the global variables of the program at the .data segment of the assembly file
Input: Backend, head of TAC instructions
Output: None
*/
void generate_global_vars(asm_backend* backend, TAC* triple) {

	table_T* table = backend->table;

	while (triple) {

		switch (triple->op) {

		case TOKEN_LBRACE:
			table->tableIndex++;
			table = table->nestedScopes[table->tableIndex - 1];
			break;

		case TOKEN_RBRACE:
			table->tableIndex = 0;
			table = table->prev;
			break;

		case AST_VARIABLE_DEC:
			if (!(table_search_table(table, triple->arg1->value)->prev)) {

				// Declaring and assigning the global var the value of the next operation (which will be an assignment)
				fprintf(backend->targetProg, "%s %s %s\n", triple->arg1->value, triple->arg2->value, triple->next->arg2->value);
				triple = triple->next;
			}
		
			break;

		}

		triple = triple->next;
	}
}

/*
write_asm is the main code generator function that produces all the Assembly code
Input: Symbol table, head of TAC list, target name for the file we want to produce
*/
void write_asm(table_T* table, TAC* head, char* targetName) {

	asm_backend* backend = init_asm_backend(table, head, targetName);	// Initialize backend
	TAC* triple = head;
	TAC* mainStart = NULL;
	int mainTableIndex = 0;

	// Print the template for the MASM Assembly program
	for (unsigned int i = 0; i < TEMPLATE_SIZE; i++) {
		fprintf(backend->targetProg, "%s\n", asm_template[i]);
	}
	
	// Generate the global variables in the .data section of the Assembly file
	fprintf(backend->targetProg, ".data\n");
	generate_global_vars(backend, triple);

	fprintf(backend->targetProg, ".code\n");

	backend->table->tableIndex = 0;

	// Main loop to generate code
	while (backend->instruction) {

		// Save main function start
		if (backend->instruction->op == AST_FUNCTION && !strcmp(backend->instruction->arg1->value, "main")) {
			mainStart = backend->instruction;
		}
			
		generate_asm(backend);
		backend->instruction = backend->instruction->next;
	}

	backend->instruction = mainStart;

	if (mainStart) {

		generate_main(backend);
	}
	else {

		printf("[Error]: No main file to start executing from");
		exit(1);
	}
		
	fclose(backend->targetProg);
	free_registers(backend);
}

/*
generate_asm checks for the operation in a TAC instruction and generates Assembly code for it
Input: Backend
Output: None
*/
void generate_asm(asm_backend* backend) {

	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	
	char* op = NULL;
	char* arg1 = NULL;
	char* arg2 = NULL;

	// We do not want to generate code for statements made outside of a function
	if (backend->instruction->op != AST_VARIABLE_DEC && backend->instruction->op != TOKEN_LBRACE
		&& backend->instruction->op != TOKEN_RBRACE && backend->instruction->op != AST_FUNCTION && !backend->table->prev) {

		return;
	}
		
	op = typeToString(backend->instruction->op);	// Get type of operation in a string form

	// Binary operations
	if (backend->instruction->op == AST_ADD || backend->instruction->op == AST_SUB) {

		generate_binop(backend);
	}
	// Multiplications and divisions are different in Assembly 8086 sadly
	else if (backend->instruction->op == AST_MUL || backend->instruction->op == AST_DIV) {

		generate_mul_div(backend);
	}
	else if (backend->instruction->op == TOKEN_LESS || backend->instruction->op == TOKEN_MORE
		|| backend->instruction->op == TOKEN_ELESS || backend->instruction->op == TOKEN_EMORE
		|| backend->instruction->op == TOKEN_DEQUAL || backend->instruction->op == TOKEN_NEQUAL) {

		generate_condition(backend);
	}
	else {

		switch (backend->instruction->op) {
		
		case TOKEN_LBRACE:
			// If we reached the start of a new block, go to the fitting symbol table for that block
			backend->table->tableIndex++;
			backend->table = backend->table->nestedScopes[backend->table->tableIndex - 1];
			break;

		case TOKEN_RBRACE: generate_block_exit(backend);

			descriptor_reset_all_registers(backend);
			// When done with a block, reset the index of the table and go to the previous one
			backend->table->tableIndex = 0;
			backend->table = backend->table->prev;
			break;

		// For each of the operation cases, generate fitting Assembly instructions
		case AST_FUNCTION: generate_function(backend); break;
		case AST_ASSIGNMENT: generate_assignment(backend); break;
		case AST_VARIABLE_DEC: generate_var_dec(backend); break;
		case AST_IFZ: generate_if_false(backend); break;
		case AST_GOTO: generate_unconditional_jump(backend); break;
		case AST_LABEL: fprintf(backend->targetProg, "%s:\n", generate_get_label(backend, backend->instruction)); descriptor_reset_all_registers(backend); break;
		case AST_LOOP_LABEL: fprintf(backend->targetProg, "%s:\n", generate_get_label(backend, backend->instruction)); descriptor_reset_all_registers(backend); break;
		case AST_FUNC_CALL: generate_func_call(backend); break;
		case AST_PRINT: generate_print(backend); break;
		case AST_RETURN: generate_return(backend); break;
	
		
		}
	}
}

/*
generate_binop generates Assembly code for addition and substruction
Input: Backend
Output: None
*/
void generate_binop(asm_backend* backend) {

	register_T* reg1 = NULL;
	register_T* reg2 = NULL;

	char* arg1 = NULL;
	char* arg2 = NULL;

	// If the operation is add or sub by 0 then we can do nothing 
	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "0")) {

		descriptor_push_tac(backend, generate_move_to_register(backend, backend->instruction->arg1), backend->instruction);
		return;
	}
	if (backend->instruction->op == AST_ADD && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "0")) {

		descriptor_push_tac(backend, generate_move_to_register(backend, backend->instruction->arg2), backend->instruction);
		return;
	}

	// Addition by 1 can be replaced by the instruction INC
	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "1")) {

		reg1 = generate_move_to_register(backend, backend->instruction->arg1);
		descriptor_push_tac(backend, reg1, backend->instruction);
		backend->instruction->op == AST_ADD ? fprintf(backend->targetProg, "INC %s\n", generate_get_register_name(reg1)) : fprintf(backend->targetProg, "DEC %s\n", generate_get_register_name(reg1));
		return;
	}
	// Substruction by 1 can be replaced by the instruction DEC
	if (backend->instruction->op == AST_ADD && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "1")) {

		reg1 = generate_move_to_register(backend, backend->instruction->arg2);
		descriptor_push_tac(backend, reg1, backend->instruction);
		fprintf(backend->targetProg, "INC %s\n", generate_get_register_name(reg1));
		return;
	}

	reg1 = generate_move_to_register(backend, backend->instruction->arg1);	// Get the first argument in a register

	arg1 = generate_assign_reg(reg1, backend->instruction->arg1->value);
	arg2 = NULL;
	
	// If the value in the second argument is a variable or temp, we want to use a register
	if (table_search_entry(backend->table, backend->instruction->arg2->value) || backend->instruction->arg2->type == TAC_P) {

		reg1->regLock = true;
		reg2 = generate_move_to_register(backend, backend->instruction->arg2);
		reg1->regLock = false;

		arg2 = generate_assign_reg(reg2, backend->instruction->arg2->value);
	}
	// If the value of the second argument is a number, we can treat it as a const instead of putting it
	// in a new register
	else {

		arg2 = backend->instruction->arg2->value;
	}

	descriptor_push_tac(backend, reg1, backend->instruction);			// We treat the whole TAC as a temporary variable that is now in the register

	fprintf(backend->targetProg, "%s %s, %s\n", typeToString(backend->instruction->op), arg1, arg2);
}

/*
generate_mul_div generates Assembly code for multiplication and division
Input: Backend
Output: None
*/
void generate_mul_div(asm_backend* backend) {

	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	entry_T* entry = NULL;

	// If one of the operands is 1 and the operation is multiplication then do nothing as multiplication by 1 means nothing
	// Also if the second argument is 1 and the operation is division we can do nothing as well for the same reason
	if (backend->instruction->arg2->type == CHAR_P && !strcmp(backend->instruction->arg2->value, "1")) {

		descriptor_push_tac(backend, generate_move_to_register(backend, backend->instruction->arg1), backend->instruction);	
		return;
	}
	if (backend->instruction->op == AST_MUL && backend->instruction->arg1->type == CHAR_P && !strcmp(backend->instruction->arg1->value, "1")) {

		descriptor_push_tac(backend, generate_move_to_register(backend, backend->instruction->arg2), backend->instruction);	
		return;
	}

	backend->registers[REG_DX]->regLock = true;		// Do not temper with DX since it can hold a carry 

	reg1 = generate_move_to_ax(backend, backend->instruction->arg1);	// Multiplication and division must use the AX register
	
	reg1->regLock = true;
	reg2 = generate_move_to_register(backend, backend->instruction->arg2);
	reg1->regLock = false;

	if (backend->instruction->op == AST_MUL) {

		fprintf(backend->targetProg, "MUL %s\n", generate_get_register_name(reg2));
	}
	else {

		// Xoring EDX is necessary because in division we divide EDX:EAX with the other register which means any 
		// value in EDX can ruin our calculations
		fprintf(backend->targetProg, "PUSH EDX\n");
		fprintf(backend->targetProg, "XOR EDX, EDX\n");
		fprintf(backend->targetProg, "DIV %s\n", generate_get_register_name(reg2));
		fprintf(backend->targetProg, "POP EDX\n");
	}

	backend->registers[REG_DX]->regLock = false;		// Remove lock on EDX after we're done
	descriptor_push_tac(backend, backend->registers[REG_AX], backend->instruction);	// Now AX will hold the result value so we can reset it to that value
}

/*
generate_condition generates Assembly code for a condition like <, == etc
Input: Backend
Output: None
*/
void generate_condition(asm_backend* backend) {

	register_T* reg1 = generate_move_to_register(backend, backend->instruction->arg1);

	reg1->regLock = true;
	register_T* reg2 = generate_move_to_register(backend, backend->instruction->arg2);
	reg1->regLock = false;

	// Generate a block exit operation because with the control flow that is occuring here,
	// we don't know if the variables changed inside a loop. For if statements we don't want the
	// change to affect our else statement
	backend->table = backend->table->nestedScopes[backend->table->tableIndex];
	generate_block_exit(backend);
	backend->table = backend->table->prev;


	fprintf(backend->targetProg, "CMP %s, %s\n", generate_get_register_name(reg1), generate_get_register_name(reg2));
	descriptor_push_tac(backend, generate_get_register(backend), backend->instruction);
}

/*
generate_if_false generates Assembly code for IFZ operations
Input: Backend
Output: None
*/
void generate_if_false(asm_backend* backend) {

	char* jmpCondition = NULL;

	if (backend->instruction->arg1->type == TAC_P || backend->instruction->arg1->type == TEMP_P) {

		// In all our cases when it comes to comparison, we want to do the exact opposite of what is specified
		switch (((TAC*)backend->instruction->arg1->value)->op) {

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
	else {
		fprintf(backend->targetProg, "CMP %s, 0\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)));
		fprintf(backend->targetProg, "JE %s\n", generate_get_label(backend, backend->instruction->arg2->value));
	}
}

/*
generate_unconditional_jump generates an Assembly code for GOTO operations
Input: Backend
Outut: None
*/
void generate_unconditional_jump(asm_backend* backend) {

	fprintf(backend->targetProg, "JMP %s\n", generate_get_label(backend, backend->instruction->arg1->value));
}

/*
generate_assignment generates Assembly code for assignments
Input: Backend
Ouput: None
*/
void generate_assignment(asm_backend* backend) {

	register_T* reg1 = NULL;
	register_T* reg2 = NULL;
	entry_T* entry = table_search_entry(backend->table, backend->instruction->arg1->value);

	if (entry->dtype == DATA_STRING) {

		fprintf(backend->targetProg, "PUSHA\n");

		// MASM macro to copy a string value onto the string array
		fprintf(backend->targetProg, "fnc lstrcpy, ADDR %s, \"%s\"\n", backend->instruction->arg1->value, backend->instruction->arg2->value);

		fprintf(backend->targetProg, "POPA\n");
	}

	// If variable equals a function call then the value will return in AX, therefore we know that the register will always be AX
	else if (backend->instruction->arg2->type == TAC_P && ((TAC*)backend->instruction->arg2->value)->op == AST_FUNC_CALL) {
		
		reg1 = backend->registers[REG_AX];
	}
	// Otherwise, without a function call, we can use any register
	else {
		
		reg1 = generate_move_to_register(backend, backend->instruction->arg2);
	}

	if (entry->dtype != DATA_STRING) {

		address_reset(entry);

		address_push(entry, reg1, ADDRESS_REG);

		// Remove variable from all registers that held it's value
		while ((reg2 = generate_check_variable_in_reg(backend, backend->instruction->arg1))) {
			generate_remove_descriptor(reg2, backend->instruction->arg1);
		}
		
		descriptor_push(reg1, backend->instruction->arg1);
	}
}

/*
generate_var_dec generates Assembly code for variable declarations
Input: Backend
Output: None
*/
void generate_var_dec(asm_backend* backend) {

	char* name = NULL;

	if (!table_search_table(backend->table, backend->instruction->arg1->value)->prev) { return; }
		

	name = backend->instruction->arg1->value;

	// If the second argument is a number, that means it's the amount of bytes to put in a string data
	// For other types, just declare them normally
	isNum(backend->instruction->arg2->value) ? fprintf(backend->targetProg, "LOCAL %s[%s]:BYTE\n", name, backend->instruction->arg2->value)
		: fprintf(backend->targetProg, "LOCAL %s:%s\n", name, backend->instruction->arg2->value);
	
}  

/*
generate_main generates Assembly code for the main function of the source code
Input: Backend
Output: None
*/
void generate_main(asm_backend* backend) {

	TAC* triple = NULL;

	char* name = backend->instruction->arg1->value;

	fprintf(backend->targetProg, "main_start:\n");
	fprintf(backend->targetProg, "CALL main\n");	// Call the actual main procedure
	fprintf(backend->targetProg, "invoke ExitProcess, 0\n");
	fprintf(backend->targetProg, "end main_start\n");
}

/*
generate_function generates Assembly code for every function but the main one, which is a procedure that returns
it's value in AX
Input: Backend
Output: None
*/
void generate_function(asm_backend* backend) {

	TAC* triple = NULL;
	arg_T* initValue = NULL;
	entry_T* entry = NULL;

	size_t variables = 0;
	size_t counter = atoi(backend->instruction->next->arg1->value);

	char* name = backend->instruction->arg1->value;
	char* varName = NULL;

	fprintf(backend->targetProg, "%s PROC ", name);	// Generating function label

	backend->table->tableIndex++;
	backend->table = backend->table->nestedScopes[backend->table->tableIndex - 1];

	// Skipping number of local vars and start of block
	backend->instruction = backend->instruction->next->next->next;

	triple = backend->instruction;

	// Generate all the local variables for the function
	if (counter > 0) {
		fprintf(backend->targetProg, "%s:%s", backend->table->entries[0]->name, dataToAsm(backend->table->entries[0]->dtype));
	}
		
	for (unsigned int i = 1; i < counter; i++) {
		fprintf(backend->targetProg, ", %s:%s", backend->table->entries[i]->name, dataToAsm(backend->table->entries[i]->dtype));
	}

	fprintf(backend->targetProg, "\n");

	// First generate only the variable declarations
	while (backend->instruction->op != TOKEN_FUNC_END) {

		if (backend->instruction->op == AST_VARIABLE_DEC) {

			if (table_search_entry(backend->table, backend->instruction->arg1->value)->dtype != DATA_STRING) {
				variables++;
			}
				
			generate_asm(backend);
		}

		backend->instruction = backend->instruction->next;
	}

	backend->instruction = triple;

	// Go and assign a starting value to each declared variable
	for (unsigned int i = 0; i < variables;)  {

		if (backend->instruction->op == AST_ASSIGNMENT 
			&& table_search_entry(backend->table, backend->instruction->arg1->value)->dtype != DATA_STRING) {

			varName = backend->instruction->arg1->value;
			initValue = backend->instruction->arg2;

			generate_asm(backend);
			fprintf(backend->targetProg, "MOV [%s], %s\n", varName, generate_get_register_name(generate_find_register(backend, initValue)));
			
			i++;
		}
				
		backend->instruction = backend->instruction->next;
	}
	
	backend->instruction = triple;

	// Loop through the function and generate code for the statements
	while (backend->instruction->op != TOKEN_FUNC_END) {

		if (backend->instruction->op != AST_VARIABLE_DEC) {
			generate_asm(backend);
		}
			
		backend->instruction = backend->instruction->next;
	}

	fprintf(backend->targetProg, "%s ENDP\n", name);
}

/*
generate_return generates Assembly code for return operations
Input: Backend
Output: None
*/
void generate_return(asm_backend* backend) {

	register_T* reg = generate_move_to_ax(backend, backend->instruction->arg1);		// Always return a value in AX
	fprintf(backend->targetProg, "RET\n");
}

/*
generate_func_call generates Assembly code for function calls
Input: Backend
Output: None
*/
void generate_func_call(asm_backend* backend) {

	char* name = backend->instruction->arg1->value;
	size_t size = atoi(backend->instruction->arg2->value);
	
	// Push all the variables to the stack, from last to first
	for (unsigned int i = 0; i < size;) {

		backend->instruction = backend->instruction->next;

		// For variables and numbers we can just push them as is
		if (backend->instruction->op == AST_PARAM && backend->instruction->arg1->type == CHAR_P) {

			fprintf(backend->targetProg, "PUSH %s\n", backend->instruction->arg1->value);
			i++;
		}
		// For TAC operations we need to allocate a register before pushing
		else if (backend->instruction->op == AST_PARAM && (backend->instruction->arg1->type == TAC_P || backend->instruction->arg1->type == TEMP_P)) {

			fprintf(backend->targetProg, "PUSH %s\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)));
			i++;
		}
		// There can be expression operations between parameters, so we need to generate code for them but make the loop longer
		else {
			generate_asm(backend);
		}
	}

	fprintf(backend->targetProg, "CALL %s\n", name);
}

/*
generate_print generates Assembly code for the built in function print
Input: Backend
Output: None
*/
void generate_print(asm_backend* backend) {

	entry_T* entry = NULL;;

	size_t size = atoi(backend->instruction->arg2->value);

	fprintf(backend->targetProg, "PUSHA\n");	// fnc will change register values, save previous values before doing so

	arg_T** regDescListList[GENERAL_REG_AMOUNT];

	// Save the register values we will change because of the macros
	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {

		regDescListList[i] = calloc(1, sizeof(arg_T));

		for (unsigned int i2 = 0; i2 < backend->registers[i]->size; i2++) {
			regDescListList[i][i2] = backend->registers[i]->regDescList[i2];
		}
	}

	// For each pushed param, produce an fnc StdOut instruction
	for (unsigned int i = 0; i < size; i++) {

		backend->instruction = backend->instruction->next;

		if (backend->instruction->op == AST_PARAM && backend->instruction->arg1->type == CHAR_P) {
			entry = table_search_entry(backend->table, backend->instruction->arg1->value);
		}

		// For data literals we just want to print them as is
		if (backend->instruction->op == AST_PARAM && !entry) {

			!(backend->instruction->arg1->type == TAC_P || backend->instruction->arg1->type == TEMP_P) ?
				fprintf(backend->targetProg, "fnc StdOut, \"%s\"\n", backend->instruction->arg1->value)
				: fprintf(backend->targetProg, "fnc StdOut, str$(%s)\n", generate_get_register_name(
					generate_move_to_register(backend, backend->instruction->arg1)));
				
		}
		// For strings being pushed, produce fitting code
		else if (backend->instruction->op == AST_PARAM && entry->dtype == DATA_STRING) {

			fprintf(backend->targetProg, "fnc StdOut, ADDR %s\n", entry->name);
		}
		// For an integer, find or allocate a register to the value and print the value using the str$ macro
		// also for temporeries
		else if (backend->instruction->op == AST_PARAM && entry->dtype == DATA_INT) {

			fprintf(backend->targetProg, "fnc StdOut, str$(%s)\n", generate_get_register_name(generate_move_to_register(backend, backend->instruction->arg1)));
		}
		else {

			generate_asm(backend);
			i--;
		}
	}

	/* Return back the values before the PUSHA instruction */

	descriptor_reset_all_registers(backend);

	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {

		for (unsigned int i2 = 0; i2 < backend->registers[i]->size; i2++) {
			descriptor_push(backend->registers[i], regDescListList[i][i2]);
		}
			
		free(regDescListList[i]);
	}

	fprintf(backend->targetProg, "POPA\n");		// Return previous values to registers in Assembly code
}

/*
generate_assign_reg returns a valid register / const number string depending on the given register
Input: A register, an argument to return if register is null
Output: A string that can either represent the given register or the given value, depending on if the register is null or not
*/
char* generate_assign_reg(register_T* r, void* argument) {
	return r ? generate_get_register_name(r) : argument;
}

/*
generate_check_variable_in_reg checks if a variable exists in any general purpose register, and if so it returns the register
Input: Register list, variable to search
Output: First register to contain the variable, NULL if none of them do
*/
register_T* generate_check_variable_in_reg(asm_backend* backend, arg_T* var) {

	register_T* reg = NULL;

	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

		for (unsigned int i2 = 0; i2 < backend->registers[i]->size; i2++) {

			if (generate_compare_arguments(var, backend->registers[i]->regDescList[i2])) {

				reg = backend->registers[i];
				break;
			}
		}
	}

	return reg;
}

/*
generate_find_free_reg finds a register that currently holds no values in it's descriptor
Input: Backend
Output: Free register if it exists, 0 if it doesn't
*/
register_T* generate_find_free_reg(asm_backend* backend) {

	register_T* reg = NULL;

	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

		if (!backend->registers[i]->size && !backend->registers[i]->regLock) {
			reg = backend->registers[i];
		}
	}

	return reg;
}

/*
generate_find_register finds a register for an argument, if the argument does not exist in any register it loads it in one
Input: Backend struct with instruction, argument to load register for
Output: Fitting register
*/
register_T* generate_find_register(asm_backend* backend, arg_T* arg) {

	register_T* reg = generate_check_variable_in_reg(backend, arg);	// Check if variable is in a register
	
	if (!reg) {
		reg = generate_get_register(backend);
	}
		
	return reg;
}

/*
generate_get_register uses all functions that try to find an available register to use and returns it
Input: Backend
Output: Available register
*/
register_T* generate_get_register(asm_backend* backend) {

	register_T* reg = generate_find_free_reg(backend);

	// Check if there's a register that can be used despite being occupied
	if (!reg) {
		reg = generate_find_used_reg(backend);		
	}
		
	return reg;
}

/*
generate_move_to_register searches for an available register and produces Assembly code to 
store a value in that register if it's not already there
Input: Backend, Argument that contains a value we want to put in a register
Output: Available register
*/
register_T* generate_move_to_register(asm_backend* backend, arg_T* arg) {

	register_T* reg = generate_check_variable_in_reg(backend, arg);		// Check if variable is in a register
	entry_T* entry = NULL;
	char* name = NULL;

	if (reg) { return reg; }
	
	entry = table_search_entry(backend->table, arg->value);	// Search for the entry

	reg = generate_get_register(backend);

	name = generate_get_register_name(reg);

	// Push the new variable descriptor onto the register
	descriptor_push(reg, arg);

	// If entry exists and value was not found in any register previously, store it in the found available register
	if (entry) {

		address_push(entry, reg, ADDRESS_REG);
		fprintf(backend->targetProg, "MOV %s, [%s]\n", name, arg->value);
	}
	// Otherwise, if we want to move a number to a register, check if that number is 0, if so generate a XOR
	// instruction, and if it isn't just load it's value onto the register
	else if (arg->type == CHAR_P) {

		!strcmp(arg->value, "0") ? fprintf(backend->targetProg, "XOR %s, %s\n", name, name)
			: fprintf(backend->targetProg, "MOV %s, %s\n", name, arg->value);	
	}
	
	return reg;
}

/*
generate_move_new_value_to_register searches for an available register and puts the desired value in it
but it doesn't search for a register that already contains the argument value
Input: Backend, Argument to find register for
Ouput: Available register
*/
register_T* generate_move_new_value_to_register(asm_backend* backend, arg_T* arg) {

	register_T* reg = NULL;
	entry_T* entry = NULL;
	char* name = NULL;

	entry = table_search_entry(backend->table, arg->value);

	reg = generate_get_register(backend);

	name = generate_get_register_name(reg);

	// Push the new variable descriptor onto the register
	descriptor_push(reg, arg);

	if (entry) {

		address_push(entry, reg, ADDRESS_REG);
		fprintf(backend->targetProg, "MOV %s, [%s]\n", name, arg->value);
	}
	else {

		!strcmp(arg->value, "0") ? fprintf(backend->targetProg, "XOR %s %s\n", name, name)
			: fprintf(backend->targetProg, "MOV %s, %s\n", name, arg->value);		
	}

	return reg;
}

/*
generate_move_to_ax moves a value only to to the AX register
Input: Backend, argument to move to AX
Output: AX register
*/
register_T* generate_move_to_ax(asm_backend* backend, arg_T* arg) {

	register_T* reg = generate_check_variable_in_reg(backend, arg);
	arg_T** regDescList = NULL;

	if (!reg) {

		// If the register found was not AX, copy AX's contents to it to free AX
		if ((reg = generate_get_register(backend))->reg != REG_AX) {

			for (unsigned int i = 0; i < backend->registers[REG_AX]->size; i++) {
				descriptor_push(reg, backend->registers[REG_AX]->regDescList[i]);
			}
			
			fprintf(backend->targetProg, "MOV %s, EAX\n", generate_get_register_name(reg));		// Move the value of AX to a different register

			// Reset AX
			backend->registers[REG_AX]->size = 0;
			free(backend->registers[REG_AX]->regDescList);
			backend->registers[REG_AX]->regDescList = NULL;
		}

		reg = generate_move_to_register(backend, arg);
	}
	else if (reg->reg != REG_AX) {

		fprintf(backend->targetProg, "XCHG EAX, %s\n", generate_get_register_name(reg));

		// Switch their register descriptors
		regDescList = backend->registers[REG_AX]->regDescList;
		backend->registers[REG_AX]->regDescList = reg->regDescList;
		reg->regDescList = regDescList;
		
	}

	return reg;
}

/*
generate_find_lowest_values finds and returns the register with the lowest amount of values stored
Input: Register list
Output: Register with lowest variables
*/
register_T* generate_find_lowest_values(asm_backend* backend) {

	size_t loc = 0;

	// Start comparing to the first available register
	while (backend->registers[loc]->regLock) { loc++; }

	// Start comparing all registers to find the lowest value
	for (unsigned int i = 1; i < GENERAL_REG_AMOUNT; i++) {

		if (backend->registers[i]->size < backend->registers[loc]->size
			&& !backend->registers[i]->regLock) {

			loc = i;
		}
	}

	return backend->registers[loc];
}

/*
generate_find_used_reg finds an available register when all registers are used
Input: Backend
Output: Newly available register
*/
register_T* generate_find_used_reg(asm_backend* backend) {

	register_T* reg = NULL;
	entry_T* entry = NULL;

	// Going through all the variables in all the registers and searching to see if there's a register
	// that has all it's values stored somewhere else as well, if so, we can use that register
	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

		if (backend->registers[i]->regLock) {
			continue;
		}
			
		reg = backend->registers[i];

		for (unsigned int i2 = 0; i2 < backend->registers[i]->size && reg; i2++) {

			// If there's a temporary in the register, we cannot use the register
			if (backend->registers[i]->regDescList[i2]->type == TAC_P 
				|| backend->registers[i]->regDescList[i2]->type == TEMP_P) {

				reg = NULL;
			}
			else if (entry = table_search_entry(backend->table, backend->registers[i]->regDescList[i2]->value)) {

				if (entry->size <= 1) {
					reg = NULL;
				}	
			}
		}
	}
	// Check that the variable the program is assigning to doesn't equal both operands
	// and if it doesn't, we can use a register that contains the variable
	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {

		if (backend->instruction->next && backend->instruction->next->op == AST_ASSIGNMENT
			&& backend->instruction->next->arg1->type != TAC_P && !backend->registers[i]->regLock) {

			reg = generate_check_useless_value(backend, backend->registers[i]);
		}
	}
	
	// Going through registers and checking if there's a register that holds values that won't be used again
	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT && !reg; i++) {
		if (backend->registers[i]->regLock) {
			continue;
		}
			
		reg = generate_check_register_usability(backend, backend->registers[i]);	
	}
	// If there are no usable registers, we need to spill the values of one of the registers
	if (!reg) {

		reg = generate_find_lowest_values(backend);
		generate_spill(backend, reg);
	}
	// Now that the variable is saved somewhere else, we need to free the previous values stored in it and reset the size
	if (reg->regDescList) {

		descriptor_reset(backend, reg);
	}
	
	return reg;
}

/*
generate_check_useless_value checks if the register holds a value that is going to disappear through an assignment instruction
Input: Backend, register to search in
Output: Returns the register if it contains useless value, NULL if it doesn't
*/
register_T* generate_check_useless_value(asm_backend* backend, register_T* r) {

	register_T* reg = r;
	
	// If we specified earlier to not use that register
	if (r->regLock || backend->instruction->next && backend->instruction->next->op != AST_ASSIGNMENT) {
		return NULL;
	}
		

	for (unsigned int i = 0; i < r->size && reg; i++) {
	
		if (generate_compare_arguments(backend->instruction->next->arg1->value, backend->instruction->arg1->value)
			|| generate_compare_arguments(backend->instruction->next->arg1->value, backend->instruction->arg2->value)
			|| !generate_compare_arguments(backend->instruction->next->arg1->value, r->regDescList[i]->value)) {
			reg = NULL;
		}
	}

	return reg;
}

/*
generate_check_register_usability goes through a given register's descriptor list to see if the values that a register holds will not be used again
Input: Backend, register to search in
Output: Returns the register if it contains unused values, NULL if not
*/
register_T* generate_check_register_usability(asm_backend* backend, register_T* r) {

	register_T* reg = NULL;

	for (unsigned int i = 0; i < r->size; i++) {
		reg = generate_check_variable_usability(backend, r, r->regDescList[i]);

		if (!reg) {
			break;
		}	
	}

	return reg;
}

/*
generate_check_variable_usability goes through a block to see if a specific value that a register holds will not be used again
Input: Backend, register to search in
Output: Returns the register if it contains an unused value, NULL if not
*/
register_T* generate_check_variable_usability(asm_backend* backend, register_T* r, arg_T* arg) {

	TAC* triple = backend->instruction;
	register_T* reg = r;

	// Loop through instructions until the end of the block to see if we can use a register that holds
	// a value that will not be used
	while (triple->op != TOKEN_RBRACE && reg) {

		// For the first argument, if we are assigning to the variable, then we are okay to use register
		if ((triple->op != AST_ASSIGNMENT && triple->arg1 && generate_compare_arguments(triple->arg1, arg))
			|| (triple->arg2 && generate_compare_arguments(triple->arg2, arg)) || reg->regLock) {
			reg = NULL;
		}
		
		triple = triple->next;
	}

	return reg;
}

/*
generate spill stores the actual value of a variable in it, after being changed by operations such as assignment
Input: Backend, register to spill variables in
Output: None
*/
void generate_spill(asm_backend* backend, register_T* r) {

	entry_T* entry = NULL;

	// For each value, store the value of the variable in itself
	for (unsigned int i = 0; i < r->size; i++) {

		if ((entry = table_search_entry(backend->table, r->regDescList[i]->value))) {

			fprintf(backend->targetProg, "MOV [%s], %s\n", r->regDescList[i]->value, generate_get_register_name(r));
			address_push(entry, r->regDescList[i]->value, ADDRESS_VAR);
		}
	}
}

/*
generate_block_exit generates Assembly code for when we exit a block, it stores all later-needed values from the registers
back to the variables
Input: Backend
Output: None
*/
void generate_block_exit(asm_backend* backend) {

	for (unsigned int i = 0; i < GENERAL_REG_AMOUNT; i++) {
		register_block_exit(backend, backend->registers[i]);
	}
}
		

/*
register_block_exit checks if a variable currently holds it's correct data, if not, it loads it before exiting a scope
Input: Backend, register to check variables in
Output: None
*/
void register_block_exit(asm_backend* backend, register_T* reg) {

	entry_T* entry = NULL;

	// For each value stored in the register, check if that value is different from what the actual variable
	// holds, if it is, store the correct value in the variable before exiting a scope
	for (unsigned int i = 0; i < reg->size; i++) {

		entry = table_search_entry(backend->table, reg->regDescList[i]->value);

		// Only check variables in registers, because only they can be live on exit
		// Also if there is no entry, we can continue searching
		if (reg->regDescList[i]->type != CHAR_P || !entry) {
			continue;
		}
			
		// For values that are live on exit (values that exist in some parent symbol table)
		if (!table_search_in_specific_table(backend->table, reg->regDescList[i]->value)
			&& !entry_search_var(entry, reg->regDescList[i]->value)) {		// Here we check if the variable doesn't hold it's own value 

			fprintf(backend->targetProg, "MOV [%s], %s\n", reg->regDescList[i]->value, generate_get_register_name(reg));
			address_remove_registers(entry);
			address_push(entry, reg->regDescList[i]->value, ADDRESS_VAR);
		}
		// We can reset the addresses for values that are not live on exit
		else if (table_search_in_specific_table(backend->table, reg->regDescList[i]->value)) {
			address_reset(entry);
		}
	}
}

/*
generate_get_label generates a label name from a TAC instruction address
Input: Backend, label instruction
Output: Label name
*/
char* generate_get_label(asm_backend* backend, TAC* label) {

	char* name = NULL;
	
	// Search for label and return it
	for (unsigned int i = 0; i < backend->labelList->size && !name; i++) {

		if (label == backend->labelList->labels[i]) {
			name = backend->labelList->names[i];
		}
	}
	// If label was not found, create a new one
	if (!name) {

		backend->labelList->labels = realloc(backend->labelList->labels, sizeof(TAC*) * ++backend->labelList->size);	// Appending list
		backend->labelList->labels[backend->labelList->size - 1] = label;
		backend->labelList->names = realloc(backend->labelList->names, sizeof(char*) * backend->labelList->size);
		name = calloc(1, strlen("label") + numOfDigits(backend->labelList->size) + 1);
		sprintf(name, "label%d", backend->labelList->size);
		backend->labelList->names[backend->labelList->size - 1] = name;
	}

	return name;
}

/*
generate_get_register_name gets a name for a register from it's type
Input: Register to get the name of
Output: Register name
*/
char* generate_get_register_name(register_T* r) {

	// Switch types
	switch (r->reg) {

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

/*
generate_compare_arguments takes two arguments and compares them according to their types
Input: First argument, second argument
Output: True if they are equal, otherwise false
*/
bool generate_compare_arguments(arg_T* arg1, arg_T* arg2) {

	bool flag = (arg1->type == TAC_P || arg1->type == TEMP_P) && (arg2->type == TAC_P || arg2->type == TEMP_P) && arg1->value == arg2->value;

	if (!flag) {
		flag = arg1->type == CHAR_P && arg2->type == CHAR_P && !strcmp(arg1->value, arg2->value);
	}
		
	return flag;
}

/*
generate_remove_descriptor removes a specific address from the register descriptor
Input: Register to remove from, address to remove
Output: None
*/
void generate_remove_descriptor(register_T* reg, arg_T* desc) {

	unsigned int index = 0;

	if (!reg) { return; }
		
	for (unsigned int i = 0; i < reg->size; i++) {

		if (!generate_compare_arguments(reg->regDescList[i], desc)) {

			reg->regDescList[index] = reg->regDescList[index];
			index++;
		}
	}

	reg->regDescList = realloc(reg->regDescList, --reg->size * sizeof(arg_T*));
}


/*
free_registers frees all the registers and their descriptors
Input: Backend
Output: None
*/
void free_registers(asm_backend* backend) {

	for (unsigned int i = 0; i < REG_AMOUNT; i++) {

		descriptor_reset(backend, backend->registers[i]);
		free(backend->registers[i]);
	}

	free(backend->registers);

	if (backend->labelList) {

		for (unsigned int i = 0; i < backend->labelList->size; i++) {
			free(backend->labelList->names[i]);
		}
			
		free(backend->labelList->names);
		free(backend->labelList->labels);
		free(backend->labelList);
	}
	
	free(backend);
}

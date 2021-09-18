#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"

#define GENERAL_REG_AMOUNT 4
#define REG_AMOUNT 11
#define TEMPLATE_SIZE 8

typedef struct REGISTER_STRUCT_STRUCT
{
	enum
	{
		REG_AX,
		REG_BX,
		REG_CX,
		REG_DX,
		REG_CS,
		REG_DS,
		REG_SS,
		REG_SP,
		REG_SI,
		REG_DI,
		REG_BP,

	} reg;

	bool regLock;			// regLock will lock the register so that get_register won't be able to use it until it's unlocked

	arg_T** regDescList;	// Register descriptors: Stores variables and addresses that have their value in the current register
	size_t size;			// Size of register descriptors

} register_T;

typedef struct LABEL_LIST_STRUCT
{
	TAC** labels;
	char** names;		// Just so I can free the allocated names later
	size_t size;

} label_list;

typedef struct ASM_BACKEND_STRUCT
{
	register_T** registers;

	label_list* labelList;

	TAC* instruction;

	table_T* table;

	FILE* targetProg;

} asm_backend;


asm_backend* init_asm_backend(table_T* table, TAC* head, char* targetName);

void write_asm(table_T* table, TAC* head, char* targetName);
void generate_asm(asm_backend* backend);
void generate_global_vars(asm_backend* backend, TAC* triple);
void descriptor_push(register_T* reg, arg_T* descriptor);
void descriptor_push_tac(register_T* reg, TAC* instruction);
void free_registers(asm_backend* registers_list);
void generate_var_dec(asm_backend* backend);
void generate_binop(asm_backend* backend);
void generate_mul_div(asm_backend* backend);
void generate_condition(asm_backend* backend);
void generate_if_false(asm_backend* backend);
void generate_unconditional_jump(asm_backend* backend);
void generate_assignment(asm_backend* backend);
void generate_block_exit(asm_backend* backend);
void generate_main(asm_backend* backend);
void generate_function(asm_backend* backend);
void generate_return(asm_backend* backend);
void generate_func_call(asm_backend* backend);
void generate_spill(asm_backend* backend, register_T* r);
void descriptor_reset(register_T* r);

register_T* generate_find_register(asm_backend* backend, arg_T* arg);
register_T* generate_move_to_ax(asm_backend* backend, arg_T* arg);
register_T* generate_move_to_register(asm_backend* backend, arg_T* arg);
register_T* generate_get_register(asm_backend* backend);
register_T* generate_check_variable_in_reg(asm_backend* backend, arg_T* var);
register_T* generate_check_useless_value(asm_backend* backend, register_T* r);
register_T* generate_find_free_reg(asm_backend* backend);
register_T* generate_find_lowest_values(asm_backend* backend);
register_T* generate_find_used_reg(asm_backend* backend);
register_T* generate_check_variable_usabilty(asm_backend* backend, register_T* r);

char* generate_get_label(asm_backend* backend, TAC* label);
char* generate_get_register_name(register_T* r);
char* generate_assign_reg(register_T* r, void* argument);

bool generate_compare_arguments(arg_T* arg1, arg_T* arg2);

#endif
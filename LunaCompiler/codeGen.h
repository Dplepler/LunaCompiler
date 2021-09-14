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

	arg_T** regDescList;	// Register descriptors: Stores variables and addresses that have their value in the current register
	size_t size;		// Size of register descriptors

} register_T;

typedef struct ASM_BACKEND_STRUCT
{
	register_T** registers;
	TAC* instruction;
	table_T* table;

	FILE* targetProg;
	unsigned int globalDataIndex;

} asm_backend;


asm_backend* init_asm_backend(table_T* table, TAC* head, char* targetName);

void generate_asm(asm_backend* backend);
void descriptor_push(register_T* reg, arg_T* descriptor);
void descriptor_push_tac(register_T* reg, TAC* instruction);
void free_registers(asm_backend* registers_list);
void write_asm(table_T* table, TAC* head, char* targetName);
void generate_var_dec(asm_backend* backend);
void generate_binop(asm_backend* backend);
void generate_mul_div(asm_backend* backend);
void generate_assignment(asm_backend* backend);
void generate_function(asm_backend* backend);
void generate_spill(asm_backend* backend, register_T* r);
void descriptor_reset(register_T* r);

register_T* generate_get_register(asm_backend* backend, arg_T* arg);
register_T* generate_check_variable_in_reg(asm_backend* backend, arg_T* var);
register_T* generate_check_useless_value(asm_backend* backend, register_T* r);
register_T* generate_find_free_reg(asm_backend* backend);
register_T* generate_find_lowest_values(asm_backend* backend);
register_T* generate_find_used_reg(asm_backend* backend);
register_T* generate_check_variable_usabilty(asm_backend* backend, register_T* r);

char* generate_get_register_name(register_T* r);
//char* dataToAsm(int type);
char* generate_assign_reg(register_T* r, void* argument);

#endif
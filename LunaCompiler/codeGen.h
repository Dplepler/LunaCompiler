#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"

#define CHAR_SIZE_OF_REG 2
#define ASM_INSTRUCTION_SIZE 3
#define GENERAL_REG_AMOUNT 4
#define REG_AMOUNT 11

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

typedef struct REGISTER_LIST_STRUCT
{
	register_T** registers;
	TAC* instruction;
	table_T* table;

} register_list;

register_list* init_registers(table_T* table, TAC* head);

void descriptor_push(register_T* reg, arg_T* descriptor);
void free_registers(register_list* registers_list);
void write_asm(table_T* table, TAC* head);
void generate_spill(register_list* registerList, register_T* r);

register_T* generate_get_register(register_list* registerList, arg_T* arg);
register_T* generate_check_variable_in_reg(register_list* registerList, void* var);
register_T* generate_check_useless_value(register_list* registerList, register_T* r);
register_T* generate_find_free_reg(register_list* registerList);
register_T* generate_find_lowest_values(register_list* registerList);
register_T* generate_find_used_reg(register_list* registerList);
register_T* generate_check_variable_usabilty(register_list* registerList, register_T* r);

char* generate_asm(register_list* registerList);
char* generate_get_register_name(register_T* r);
char* generate_assign_reg(register_T* r, void* argument);

#endif
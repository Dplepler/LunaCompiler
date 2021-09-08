#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"

#define REG_AMOUNT 11
#define GENERAL_REG_AMOUNT 4
#define CHAR_SIZE_OF_REG 2

typedef struct REGISTER_STRUCT
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

	char** regDesc;		// Register descriptor: Stores variables that have their value in the current register
	size_t size;		// Size of register descriptors

}register_T;

typedef struct REGISTER_LIST_STRUCT
{
	register_T** registers;
	TAC* instruction;
	table_T* table;

}register_list;

register_list* init_registers();

void push_descriptor(register_T* reg, char* descriptor);
void free_registers(register_list* registers_list);
void write_asm();
void generate_asm(register_list* registerList);
void generate_spill(register_list* registerList, register_T* r);

register_T* generate_check_variable_in_reg(register_list* registerList, char* var);
register_T* generate_find_free_reg(register_list* registerList);
register_T* generate_find_lowest_values(register_list* registerList);
register_T* generate_find_used_reg(register_list* registerList);
register_T* generate_check_variable_usabilty(register_list* registerList, register_T* r);

char* generate_get_register(register_list* registerList, void* arg);
char* generate_get_register_name(register_T* r);


#endif
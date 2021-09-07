#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"

#define REG_AMOUNT 11
#define GENERAL_REG_AMOUNT 4

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


#endif
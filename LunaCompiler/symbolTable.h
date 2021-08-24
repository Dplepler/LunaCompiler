#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "io.h"

typedef struct STRUCT_SYMBOL_ENTRY
{
	char* name;
	enum
	{
		DATA_INT,

	}dtype;

	struct STRUCT_SYMBOL_TABLE* next;

}entry_T;

typedef struct STRUCT_SYMBOL_TABLE
{
	entry_T** entries;
	table_T* next;
	table_T* prev;

}table_T;

entry_T* init_entry(char* name, int type);


#endif
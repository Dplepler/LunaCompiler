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


}entry_T;

typedef struct STRUCT_SYMBOL_TABLE
{
	entry_T** entries;

	struct STRUCT_SYMBOL_TABLE** nestedScopes;
	struct STRUCT_SYMBOL_TABLE* prev;
	
	size_t entry_size;
	size_t nested_size;

}table_T;

entry_T* init_entry(char* name, int type);
void table_add_entry(table_T* table, char* name, int type);
table_T* init_table(table_T* prev);
table_T* table_add_table(table_T* table);
entry_T* table_search_entry(table_T* table, char* name);
void table_print_table(table_T* table, int level);

#endif

#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "io.h"

typedef enum data_type
{
	DATA_INT = 45,			// Continuation from AST

}dtype;

typedef struct STRUCT_SYMBOL_ENTRY
{
	char* name;
	dtype dtype;

	void** addressDesc;	// Address descriptor: Keeping track of which addresses keep the current value of entry
	size_t size;		// Size of addresses in array


}entry_T;

typedef struct STRUCT_SYMBOL_TABLE
{
	entry_T** entries;

	struct STRUCT_SYMBOL_TABLE** nestedScopes;
	struct STRUCT_SYMBOL_TABLE* prev;
	
	size_t entrySize;
	size_t nestedSize;
	size_t tableIndex;

}table_T;



entry_T* init_entry(char* name, int type);
entry_T* table_search_entry(table_T* table, char* name);

table_T* init_table(table_T* prev);
table_T* table_add_table(table_T* table);
table_T* table_search_table(table_T* table, char* name);

void address_push(entry_T* entry, void* location);
void address_reset(entry_T* entry);
void table_add_entry(table_T* table, char* name, int type);
void table_print_table(table_T* table, int level);
void table_free_table(table_T* table);


#endif

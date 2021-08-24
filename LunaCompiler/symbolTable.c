#include "symbolTable.h"

entry_T* init_entry(char* name, int type)
{
	entry_T* entry = (entry_T*)calloc(1, sizeof(entry_T));
	entry->name = name;
	entry->dtype = type;
	
	entry->next = NULL;

	return entry;
}

table_T* init_table(table_T* prev)
{
	table_T* table = (table_T*)calloc(1, sizeof(table_T));
	table->prev = prev;
}

entry_T* table_search_entry(table_T* table, char* name)
{
	
}
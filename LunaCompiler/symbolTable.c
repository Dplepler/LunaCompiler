#include "symbolTable.h"

entry_T* init_entry(char* name, int type)
{
	entry_T* entry = calloc(1, sizeof(entry_T));
	entry->name = name;
	entry->dtype = type;

	return entry;
}

table_T* init_table(table_T* prev)
{
	table_T* table = calloc(1, sizeof(table_T));
	table->prev = prev;

	table->entries = calloc(1, sizeof(entry_T*));
	table->nestedScopes = calloc(1, sizeof(table_T*));

	return table;
}

void table_add_entry(table_T* table, char* name, int type)
{
	table->entries = realloc(table->entries, sizeof(entry_T*) * ++table->entry_size);
	table->entries[table->entry_size - 1] = init_entry(name, type);
}

table_T* table_add_table(table_T* table)
{	
	table->nestedScopes = realloc(table->nestedScopes, sizeof(table_T*) * ++table->nested_size);
	table->nestedScopes[table->nested_size - 1] = init_table(table);

	return table->nestedScopes[table->nested_size - 1];
}

entry_T* table_search_entry(table_T* table, char* name)
{
	unsigned int i = 0;
	entry_T* entry = NULL;

	for (i = 0; i < table->entry_size && !entry; i++)
	{
		if (!strcmp(table->entries[i]->name, name))
			entry = table->entries[i];
	}
		
	if (!entry && table->prev)
		entry = table_search_entry(table->prev, name);

	return entry;
}

void table_print_table(table_T* table, int level)
{
	unsigned int i = 0;

	if (table)
	{
		for (i = 0; i < table->entry_size; i++)
		{
			printf("[Level]: %d, [Entry]: %s\n", level, table->entries[i]->name);
		}
		for (i = 0; i < table->nested_size; i++)
		{
			table_print_table(table->nestedScopes[i], level + 1);
		}
	}
}

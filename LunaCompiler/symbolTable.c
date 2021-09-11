#include "symbolTable.h"

entry_T* init_entry(char* name, int type)
{
	entry_T* entry = calloc(1, sizeof(entry_T));
	entry->name = name;
	entry->dtype = type;

	entry->addressDesc = calloc(1, sizeof(void*));

	address_push(entry, entry->name);

	return entry;
}

void address_push(entry_T* entry, void* location)
{
	entry->addressDesc = realloc(entry->addressDesc, sizeof(location) * ++entry->size);
	entry->addressDesc[entry->size - 1] = location;
}

void address_reset(entry_T* entry)
{
	if (entry->addressDesc)
	{
		free(entry->addressDesc);
		entry->addressDesc = NULL;
		entry->size = 0;
	}
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
	table->entries = realloc(table->entries, sizeof(entry_T*) * ++table->entrySize);
	table->entries[table->entrySize - 1] = init_entry(name, type);
}

table_T* table_add_table(table_T* table)
{	
	table->nestedScopes = realloc(table->nestedScopes, sizeof(table_T*) * ++table->nestedSize);
	table->nestedScopes[table->nestedSize - 1] = init_table(table);

	return table->nestedScopes[table->nestedSize - 1];
}

entry_T* table_search_entry(table_T* table, char* name)
{
	unsigned int i = 0;
	entry_T* entry = NULL;

	for (i = 0; i < table->entrySize && !entry; i++)
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
	unsigned int i2 = 0;

	if (table)
	{
		for (i = 0; i < table->entrySize; i++)
		{
			printf("[Level]: %d, [Entry]: %s\n", level, table->entries[i]->name);
			printf("Address descriptors\n");
			for (i2 = 0; i2 < table->entries[i]->size; i2++)
			{
				printf("%s\n", table->entries[i]->addressDesc[i2]);
			}
		}
		for (i = 0; i < table->nestedSize; i++)
		{
			table_print_table(table->nestedScopes[i], level + 1);
		}
	}
}

/*
table_free_table is a postorder tree traversal that frees all nodes of the symbol table
Input: Root of table tree
Output: None
*/
void table_free_table(table_T* table)
{
	unsigned int i = 0;

	if (table)
	{
		for (i = 0; i < table->nestedSize; i++)
		{
			table_free_table(table->nestedScopes[i]);
		}
		for (i = 0; i < table->entrySize; i++)
		{
			free(table->entries[i]->addressDesc);
			free(table->entries[i]);
		}

		free(table->entries);
		free(table->nestedScopes);
		free(table);
	}

}
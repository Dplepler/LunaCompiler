#include "symbolTable.h"

entry_T* init_entry(char* name, int type)
{
	entry_T* entry = calloc(1, sizeof(entry_T));
	entry->name = name;
	entry->dtype = type;

	entry->addressDesc = calloc(1, sizeof(void*));

	address_push(entry, entry->name, ADDRESS_VAR);

	return entry;
}

void address_push(entry_T* entry, void* location, int type)
{
	entry->addressDesc = realloc(entry->addressDesc, sizeof(address_T*) * ++entry->size);
	entry->addressDesc[entry->size - 1] = calloc(1, sizeof(address_T));
	entry->addressDesc[entry->size - 1]->address = location;
	entry->addressDesc[entry->size - 1]->type = type;
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

/*
table_search_entry searches which scope does a variable belong to, going from current scope to it's parents until we reach the global scope
Input: Table, variable to search for
Output: Entry that contains the variable
*/
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

/*
table_search_table searches an entry just like the above function "table_search_entry" but returns the table instead of the entry
Input: Table, variable name
Output: Table that contains the variable
*/
table_T* table_search_table(table_T* table, char* name)
{
	unsigned int i = 0;

	for (i = 0; i < table->entrySize; i++)
	{
		if (!strcmp(table->entries[i]->name, name))
			return table;
	}

	if (table->prev)
		table_search_table(table->prev, name);

	return NULL;
}

bool table_search_in_specific_table(table_T* table, char* entry)
{
	bool flag = false;

	unsigned int i = 0;

	for (i = 0; i < table->entrySize && !flag; i++)
	{
		if (!strcmp(table->entries[i]->name, entry))
			flag = true;
	}

	return flag;
}

bool table_search_address(entry_T* entry, char* name)
{
	bool flag = false;
	unsigned int i = 0;

	
	for (i = 0; entry && i < entry->size && !flag; i++)
	{
		if (entry->addressDesc[i]->type == ADDRESS_VAR)
		{
			if (!strcmp(entry->addressDesc[i]->address, name))
				flag = true;

		}
	}
	
	

	return flag;
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
				printf("%s\n", table->entries[i]->addressDesc[i2]->address);
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
#include "symbolTable.h"

/*
init_entry initializes an entry with an entry name and data type
Input: Name of entry, data type of entry (e.g: int, string)
Output: Entry
*/
entry_T* init_entry(char* name, int type)
{
	entry_T* entry = calloc(1, sizeof(entry_T));
	entry->name = name;
	entry->dtype = type;

	entry->addressDesc = calloc(1, sizeof(void*));

	address_push(entry, entry->name, ADDRESS_VAR);

	return entry;
}

/*
address_push pushes an address to the address descriptor field of each entry in the symbol table
Input: Entry to push to, the address location to push and it's type
Output: None
*/
void address_push(entry_T* entry, void* location, int type)
{
	entry->addressDesc = realloc(entry->addressDesc, sizeof(address_T*) * ++entry->size);
	entry->addressDesc[entry->size - 1] = calloc(1, sizeof(address_T)); 
	entry->addressDesc[entry->size - 1]->address = location;
	entry->addressDesc[entry->size - 1]->type = type;
}

/*
address_reset resets the address descriptor of an entry
Input: Entry
Output: None
*/
void address_reset(entry_T* entry)
{
	unsigned int i = 0;

	if (!entry->addressDesc)
		return;

	// Go through and free all addresses stored in the descriptor
	for (i = 0; i < entry->size; i++)
	{
		free(entry->addressDesc[i]);
		entry->addressDesc[i] = NULL;
	}
			
	// Free descriptor
	free(entry->addressDesc);
	entry->addressDesc = NULL;
	entry->size = 0;
	
}

/*
address_remove_registers removes all descriptors of type register from an entry
Input: Entry to remove registers from
Output: None
*/
void address_remove_registers(entry_T* entry)
{
	unsigned int i = 0;
	size_t size = entry->size;

	if (!entry->addressDesc)
		return;

	for (i = 0; i < size; i++)
	{
		if (entry->addressDesc[i]->type != ADDRESS_REG)
			continue;

		entry->size--;
		free(entry->addressDesc[i]);
		entry->addressDesc[i] = NULL;
		
	}
}

/*
address_remove_register removes a specific register from a variable descriptor
Input: Entry to remove register from, address of register to remove
Output: None
*/
void address_remove_register(entry_T* entry, void* reg)
{
	unsigned int i = 0;
	size_t size = entry->size;

	if (!entry->addressDesc)
		return;

	for (i = 0; i < size; i++)
	{
		if (entry->addressDesc[i]->type != ADDRESS_REG || entry->addressDesc[i]->address != reg)
			continue;
			
		entry->size--;
		free(entry->addressDesc[i]);
		entry->addressDesc[i] = NULL;
		break;
	}
}

/*
init_table initializes a symbol table with a given parent
Input: The parent table of the desired new table
Output: Initialized table
*/
table_T* init_table(table_T* prev)
{
	table_T* table = calloc(1, sizeof(table_T));
	table->prev = prev;

	table->entries = calloc(1, sizeof(entry_T*));
	table->nestedScopes = calloc(1, sizeof(table_T*));

	return table;
}

/*
table_add_entry adds an entry to a table
Input: Table to add entry to, entry name and type
*/
void table_add_entry(table_T* table, char* name, int type)
{
	table->entries = realloc(table->entries, sizeof(entry_T*) * ++table->entrySize);
	table->entries[table->entrySize - 1] = init_entry(name, type);
}

/*
table_add_table adds a table to a given table as part of it's nested scopes
Input: Table to add to
Input: New table
*/
table_T* table_add_table(table_T* table)
{	
	table->nestedScopes = realloc(table->nestedScopes, sizeof(table_T*) * ++table->nestedSize);
	table->nestedScopes[table->nestedSize - 1] = init_table(table);

	return table->nestedScopes[table->nestedSize - 1];
}

/*
table_search_entry searches which scope does a variable belong to, going from current scope to it's parents until we reach the global scope
Input: Table, variable to search for
Output: Entry that contains the variable, 0 if non of them do
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
		
	// If entry was not found, go to the parent table
	if (!entry && table->prev)
		entry = table_search_entry(table->prev, name);

	return entry;
}

/*
table_search_table searches an entry just like the above function "table_search_entry" but returns the table instead of the entry
Input: Table to start searching in, variable name
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

/*
table_search_in_specific_table searches an entry but only in the specific specified table
Input: Table to search in, entry to search
Output: True if entry was found, false if it wasn't
*/
bool table_search_in_specific_table(table_T* table, char* entry)
{
	bool flag = false;

	unsigned int i = 0;

	for (i = 0; i < table->entrySize && !flag; i++)
		flag = !strcmp(table->entries[i]->name, entry);
	

	return flag;
}

/*
table_search_address searches an address in an entry
Input: Entry to search in, address name to search for
Output: True if found, otherwise false
*/
bool table_search_address(entry_T* entry, char* name)
{
	bool flag = false;
	unsigned int i = 0;

	// Go through all addresses of entry
	for (i = 0; entry && i < entry->size && !flag; i++)
		flag = entry->addressDesc[i]->type == ADDRESS_VAR && !strcmp(entry->addressDesc[i]->address, name);
	
	
	return flag;
}

/*
table_print_table recursively prints the symbol table with levels
Input: Root of the table, level to start printing from
Output: None
*/
void table_print_table(table_T* table, int level)
{
	unsigned int i = 0;
	unsigned int i2 = 0;

	if (!table)
		return;
	
	for (i = 0; i < table->entrySize; i++)
	{
		printf("[Level]: %d, [Entry]: %s\n", level, table->entries[i]->name);
		printf("Address descriptors\n");

		for (i2 = 0; i2 < table->entries[i]->size; i2++)
			printf("%s\n", table->entries[i]->addressDesc[i2]->address);
		
	}
	for (i = 0; i < table->nestedSize; i++)
		table_print_table(table->nestedScopes[i], level + 1);
	
}


/*
table_free_table is a postorder tree traversal that frees all nodes of the symbol table
Input: Root of table tree
Output: None
*/
void table_free_table(table_T* table)
{
	unsigned int i = 0;
	unsigned int i2 = 0;

	if (!table)
		return;
	
	for (i = 0; i < table->nestedSize; i++)
		table_free_table(table->nestedScopes[i]);
	

	for (i = 0; i < table->entrySize; i++)
	{
		for (i2 = 0; i2 < table->entries[i]->size; i2++)
			if (table->entries[i]->addressDesc[i2]) free(table->entries[i]->addressDesc[i2]); 
			
		free(table->entries[i]->addressDesc);
		free(table->entries[i]);
	}

	free(table->entries);
	free(table->nestedScopes);
	free(table);
	
}
 
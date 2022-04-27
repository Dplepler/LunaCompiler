#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "io.h"

typedef enum data_type {

  DATA_INT = 48,      // Continuation from AST
  DATA_STRING,

} dtype;

typedef struct ADDRESS_STRUCT {

  void* address;
  enum ADDRESS_TYPE_ENUM {

    ADDRESS_REG,
    ADDRESS_VAR,

  } type;

} address_T;

typedef struct STRUCT_SYMBOL_ENTRY {

  char* name;
  char* value;
  dtype dtype;

  address_T** addressDesc;  // Address descriptor: Keeping track of which addresses keep the current value of entry
  size_t size;              // Size of addresses in array

} entry_T;

typedef struct STRUCT_SYMBOL_TABLE {

  entry_T** entries;

  struct STRUCT_SYMBOL_TABLE** nestedScopes;
  struct STRUCT_SYMBOL_TABLE* prev;
  
  size_t entrySize;
  size_t nestedSize;
  size_t tableIndex;

} table_T;



entry_T* init_entry(char* name, int type);
entry_T* table_search_entry(table_T* table, char* name);

table_T* init_table(table_T* prev);
table_T* table_add_table(table_T* table);
table_T* table_search_table(table_T* table, char* name);

bool table_search_in_specific_table(table_T* table, char* entry);
bool entry_search_var(entry_T* entry, char* name);

void address_push(entry_T* entry, void* location, int type);
void address_reset(entry_T* entry);
void address_remove_registers(entry_T* entry);
void address_remove_register(entry_T* entry, void* reg);
void table_add_entry(table_T* table, char* name, int type);
void table_print_table(table_T* table, int level);
void table_free_table(table_T* table);
void table_add_builtin_functions(table_T* table);

#endif

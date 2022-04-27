#ifndef CODEGEN_H
#define CODEGEN_H

#include "TAC.h"
#include <stdint.h>

#define GENERAL_REG_AMOUNT 4
#define REG_AMOUNT 11
#define TEMPLATE_SIZE 13

typedef struct REGISTER_STRUCT_STRUCT {

  enum REG_ENUM {

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

  bool regLock;      // regLock will lock the register so that get_register won't be able to use it until it's unlocked

  arg_T** regDescList;  // Register descriptors: Stores variables and addresses that have their value in the current register
  size_t size;      // Size of register descriptors

} register_T;

typedef struct LABEL_LIST_STRUCT {

  TAC** labels;
  char** names;    // Just so I can free the allocated names later
  size_t size;

} label_list;

typedef struct ASM_BACKEND_STRUCT {

  register_T** registers;

  label_list* labelList;

  TAC* instruction;

  table_T* table;

  FILE* targetProg;

} asm_frontend;

asm_frontend* init_asm_frontend(table_T* table, TAC* head, char* targetName);

void write_asm(table_T* table, TAC* head, char* targetName);
void generate_asm(asm_frontend* frontend);
void generate_global_vars(asm_frontend* frontend, TAC* triple);
void descriptor_push(register_T* reg, arg_T* descriptor);
void descriptor_push_tac(asm_frontend* frontend, register_T* reg, TAC* instruction);
void free_registers(asm_frontend* registers_list);
void generate_var_dec(asm_frontend* frontend);
void generate_binop(asm_frontend* frontend);
void generate_mul_div(asm_frontend* frontend);
void generate_condition(asm_frontend* frontend);
void generate_if_false(asm_frontend* frontend);
void generate_unconditional_jump(asm_frontend* frontend);
void generate_asm_block(asm_frontend* frontend);
void generate_assignment(asm_frontend* frontend);
void generate_block_exit(asm_frontend* frontend);
void generate_main(asm_frontend* frontend);
void generate_function(asm_frontend* frontend);
void generate_return(asm_frontend* frontend);
void generate_func_call(asm_frontend* frontend);
void generate_print(asm_frontend* frontend);
void generate_spill(asm_frontend* frontend, register_T* r);
void descriptor_reset(asm_frontend* frontend, register_T* r);
void descriptor_reset_all_registers(asm_frontend* frontend);
void generate_remove_descriptor(register_T* reg, arg_T* desc);
void restore_save_registers(asm_frontend* frontend);
void generate_free_ax(asm_frontend* frontend, register_T* reg);

register_T* generate_find_register(asm_frontend* frontend, arg_T* arg);
register_T* generate_move_to_ax(asm_frontend* frontend, arg_T* arg);
register_T* generate_move_to_register(asm_frontend* frontend, arg_T* arg);
register_T* generate_move_new_value_to_register(asm_frontend* frontend, arg_T* arg);
register_T* generate_get_register(asm_frontend* frontend);
register_T* generate_check_variable_in_reg(asm_frontend* frontend, arg_T* var);
register_T* generate_check_useless_value(asm_frontend* frontend, register_T* r);
register_T* generate_find_free_reg(asm_frontend* frontend);
register_T* generate_find_lowest_values(asm_frontend* frontend);
register_T* generate_find_used_reg(asm_frontend* frontend);
register_T* generate_check_register_usability(asm_frontend* frontend, register_T* r);
register_T* generate_check_variable_usability(asm_frontend* frontend, register_T* r, arg_T* arg);

char* generate_get_label(asm_frontend* frontend, TAC* label);
char* generate_get_register_name(register_T* r);
char* generate_assign_reg(register_T* r, void* argument);

bool generate_check_free_register(register_T* reg);
bool generate_compare_arguments(arg_T* arg1, arg_T* arg2);

void register_block_exit(asm_frontend* frontend, register_T* reg);

#endif
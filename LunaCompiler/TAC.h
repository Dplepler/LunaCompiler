#ifndef TAC_H
#define TAC_H
#include "parser.h"

typedef struct TAC_STRUCT
{
	int op;
	void* arg1;
	void* arg2;

}TAC;

typedef struct INSTRUCTIONS_STRUCT
{
	TAC** instructions;
	size_t size;

}TAC_list;

TAC_list* init_tac_list();
TAC_list* traversal_visit(AST* node);
TAC* traversal_binop(AST* node, TAC_list* list);
TAC* traversal_function_call(AST* node, TAC_list* list);
TAC* traversal_func_dec(AST* node, TAC_list* list);
TAC* traversal_assignment(AST* node, TAC_list* list);
void* traversal_build_instruction(AST* node, TAC_list* list);
void traversal_statements(AST* node, TAC_list* list);
void list_push(TAC_list* list, TAC* instruction);


#endif
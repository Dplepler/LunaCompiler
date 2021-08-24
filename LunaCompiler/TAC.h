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
TAC** traversal_visit(AST* node);
TAC* traversal_binop(AST* node, TAC_list* list);
void traversal_build_instruction(AST* node, TAC_list* list);
void* traversal_build_argument(AST* node, TAC_list* list);
void list_push(TAC_list* list, TAC* instruction);

#endif
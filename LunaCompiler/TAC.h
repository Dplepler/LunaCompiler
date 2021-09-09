#ifndef TAC_H
#define TAC_H
#include "parser.h"

// This is a tagged union to determine if args of TAC will point to a struct or to a string
typedef struct ARG_STRUCT
{
	void* arg;
	// Type of arg, TAC or string
	enum
	{
		TAC_P,
		CHAR_P,
	}type;

}arg_T;

typedef struct TAC_STRUCT
{
	int op;
	arg_T* arg1;
	arg_T* arg2;

	
	struct TAC_STRUCT* next;

}TAC;

typedef struct INSTRUCTIONS_STRUCT
{
	TAC* head;
	TAC* last;
	size_t size;

}TAC_list;

TAC_list* init_tac_list();
TAC_list* traversal_visit(AST* node);
arg_T* init_arg(void* arg, int type);
int traversal_check_arg(AST* node);
TAC* traversal_binop(AST* node, TAC_list* list);
TAC* traversal_function_call(AST* node, TAC_list* list);
TAC* traversal_func_dec(AST* node, TAC_list* list);
TAC* traversal_assignment(AST* node, TAC_list* list);
TAC* traversal_return(AST* node, TAC_list* list);
void traversal_if(AST* node, TAC_list* list);
void traversal_while(AST* node, TAC_list* list);
void* traversal_build_instruction(AST* node, TAC_list* list);
void traversal_statements(AST* node, TAC_list* list);
void list_push(TAC_list* list, TAC* instruction);
void traversal_free_array(TAC_list* list);
void traversal_print_instructions(TAC_list* instructions);


#endif

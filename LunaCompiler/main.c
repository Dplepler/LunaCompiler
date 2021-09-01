#include "TAC.h"
#define SIZE 100

void free_AST(AST* node);
//void free_tokens(token_list* list);

int main(int argc, char** argv)
{
	FILE* file = NULL;
	lexer_T* lexer = NULL;
	token_T* token = NULL;
	parser_T* parser = NULL;
	AST* root = NULL;
	TAC* triple = NULL;
	TAC_list* instructions = NULL;

	unsigned int i = 0;

	char* contents = NULL;

	if (file = fopen(argv[1], "r"))
	{
		contents = read_file(file);

		if (contents)
		{
			lexer = init_lexer(contents);

			parser = init_parser(lexer);
			root = parser_parse(parser);
			instructions = traversal_visit(root);

			triple = instructions->head;

			for (i = 0; i < instructions->size; i++)
			{
				if (triple->op)
					printf("Operation: %s, ", typeToString(triple->op));
				printf("Arg1: %s (%p), ", (char*)triple->arg1, (TAC*)triple->arg1);
				if ((char*)triple->arg2)
					printf("Arg2: %s (%p), ", (char*)triple->arg2, (TAC*)triple->arg2);
				printf("Address: %p\n", (char*)triple);

				triple = triple->next;
			}

			table_print_table(parser->table, 0);
			free_AST(root);
			//free_tokens(parser->lexer->list);
			free(contents);
			
		}
	}
	else
		printf("File does not exist\n");
	
	printf("test \n\n");
	return 0;
}

void free_AST(AST* node)
{
	unsigned int i = 0;

	if (node->type == AST_ADD || node->type == AST_SUB || node->type == AST_MUL || node->type == AST_DIV 
		|| node->type == AST_COMPARE || node->type == AST_ASSIGNMENT)
	{
		if (node->value)
		{
			free_AST(node->value);
		}
		else
		{
			free_AST(node->leftChild);
			free_AST(node->rightChild);
		}
		free(node);
	}
	else if (node->type == AST_INT || node->type == AST_VARIABLE)
	{
		free(node);
	}
	else if (node->type == AST_IF || node->type == AST_WHILE)
	{
		free_AST(node->if_body);
		free_AST(node->condition);
		if (node->else_body)
			free_AST(node->else_body);
		free(node);
	}
	else if (node->type == AST_VARIABLE_DEC || node->type == AST_RETURN)
	{
		free(node->value);
		free(node);
	}

	else
	{
		switch (node->type)
		{
			case AST_PROGRAM:
				for (i = 0; i < node->size; i++)
					free_AST(node->function_list[i]);
				free(node->function_list);
				free(node);
				break;

			case AST_FUNCTION:
				for (i = 0; i < node->size; i++)
					free_AST(node->function_def_args[i]);
				free(node->function_def_args);
				free(node);
				break;

			case AST_COMPOUND:
				for (i = 0; i < node->size; i++)
					free_AST(node->children[i]);
				free(node->children);
				free(node);
				break;

			case AST_FUNC_CALL:
				for (i = 0; i < node->size; i++)
					free_AST(node->arguments[i]);
				free(node->arguments);
				free(node);
				break;

		}
	}
}

//void free_tokens(token_list* list)
//{
//	unsigned int i = 0;
//
//	for (i = 0; i < list->size - 1; i++)
//	{
//		printf("Token: Hello\n");
//	}
//}

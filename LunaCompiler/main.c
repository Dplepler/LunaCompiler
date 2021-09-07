
#include "TAC.h"
#define SIZE 100




int main(int argc, char** argv)
{
	FILE* file = NULL;
	lexer_T* lexer = NULL;
	token_T* token = NULL;
	parser_T* parser = NULL;
	AST* root = NULL;
	TAC* triple = NULL;
	TAC_list* instructions = NULL;

	char* test = "Hello";

	unsigned int i = 0;

	instructions = init_tac_list();

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

			table_print_table(parser->table, 0);

			for (i = 0; i < instructions->size; i++)
			{
				if (triple->op)
					printf("Operation: %s, ", typeToString(triple->op));
				if (triple->arg1)
					printf("Arg1: %s (%p), ", triple->arg1, triple->arg1);
				if (triple->arg2)
					printf("Arg2: %s (%p), ", triple->arg2, triple->arg2);
				printf("Address: %p\n", triple);
				
				triple = triple->next;
			}

			lexer_free_tokens(lexer);
			AST_free_AST(root);
			traversal_free_array(instructions);
			table_free_table(parser->table);
			free(contents);
			free(lexer);
			free(parser);

		}
	}
	else
		printf("File does not exist\n");
	
	_CrtDumpMemoryLeaks();
	return 0;
}

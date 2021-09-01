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

			AST_free_AST(root);
			lexer_free_tokens();
			traversal_free_array(instructions);

			free(contents);
			
		}
	}
	else
		printf("File does not exist\n");
	
	printf("test \n\n");
	return 0;
}





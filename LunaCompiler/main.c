
#include "codeGen.h"
#define SIZE 100

int main(int argc, char** argv)
{
	FILE* file = NULL;
	lexer_T* lexer = NULL;
	token_T* token = NULL;
	parser_T* parser = NULL;
	AST* root = NULL;
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
		

			//table_print_table(parser->table, 0);

			//traversal_print_instructions(instructions);

			write_asm(parser->table, instructions->head);


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

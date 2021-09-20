
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

	char* command = NULL;
	char* fileChoice = NULL;

	unsigned int i = 0;

	char* contents = NULL;
	if (!argv[1] || !argv[2])
	{
		printf("Some file input is missing\n");
		exit(1);
	}
	
	if (!strcmp(argv[2], "-h"))
	{
		command = calloc(1, strlen("python translator.py ") + strlen(argv[1]) + 1);
		sprintf(command, "python translator.py %s", argv[1]);
		system(command);

		fileChoice = calloc(1, strlen(argv[1]) + strlen("translated_") + 1);
		sprintf(fileChoice, "translated_%s", argv[1]);

		free(command);
	}
	else
	{
		fileChoice = calloc(1, strlen(argv[1]) + 1);
		strcpy(fileChoice, argv[1]);
	}

	if (file = fopen(fileChoice, "r"))
	{
		contents = read_file(file);

		if (!contents)
		{
			printf("[ERROR]: Couldn't read file");
			exit(1);
		}
		
		lexer = init_lexer(contents);

		parser = init_parser(lexer);
		root = parser_parse(parser);
		instructions = traversal_visit(root);
	
		//table_print_table(parser->table, 0);

		traversal_print_instructions(instructions);
			
		write_asm(parser->table, instructions->head, make_new_filename(argv[1]));



		lexer_free_tokens(lexer);
		AST_free_AST(root);
		traversal_free_array(instructions);
		table_free_table(parser->table);
		free(contents);
		free(lexer);
		free(parser);

		if (!strcmp(argv[2], "-h"))
			remove(fileChoice);

		free(fileChoice);

		
	}
	else
		printf("File does not exist\n");
	
	_CrtDumpMemoryLeaks();
	return 0;
}

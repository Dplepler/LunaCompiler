
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
	char* newFilename = make_new_filename(argv[1], ".asm");

	char* contents = NULL;

	// Raise error if user didn't input filename or compile mode
	if (!argv[1] || !argv[2])
	{
		printf("[Error]: Some file input is missing");
		exit(1);
	}
	
	// If user wants to compile in Hebrew, run the translator and replace the file with the translated file
	if (!strcmp(argv[2], "-h"))
	{
		command = calloc(1, strlen("python translator.py ") + strlen(argv[1]) + 1);
		sprintf(command, "python translator.py %s", argv[1]);
		system(command);

		fileChoice = calloc(1, strlen(argv[1]) + strlen("translated_") + 1);
		sprintf(fileChoice, "translated_%s", argv[1]);

		free(command);
	}
	// Otherwise, we just want to use the normal filename
	else
	{
		fileChoice = calloc(1, strlen(argv[1]) + 1);
		strcpy(fileChoice, argv[1]);
	}

	if (file = fopen(fileChoice, "r"))		// Read file
	{
		contents = read_file(file);			// Read contents of file

		if (!contents)
		{
			printf("[Error]: Couldn't read file contents");
			exit(1);
		}
		
		lexer = init_lexer(contents);		// Initialize lexer
		parser = init_parser(lexer);		// Initialize Parser

		root = parser_parse(parser);		// Parse the tokens into an AST

		instructions = traversal_visit(root);	// Visit the AST and generate an intermidiate representation
	
		//table_print_table(parser->table, 0);
		traversal_print_instructions(instructions);
			
		// Write the Assembly code from the given IR
		write_asm(parser->table, instructions->head, newFilename);

		assemble_file(newFilename);

		// Free everything
		lexer_free_tokens(lexer);
		AST_free_AST(root);
		traversal_free_array(instructions);
		table_free_table(parser->table);
		free(contents);
		free(lexer);
		free(parser->reserved);
		free(parser);

		// If we made a new file for the translated version from Hebrew, delete that file
		if (!strcmp(argv[2], "-h"))
			remove(fileChoice);

		free(newFilename);
		free(fileChoice);
	}
	else
		printf("File does not exist\n");
	
	_CrtDumpMemoryLeaks();
	return 0;
}

#include "codeGen.h"
#define SIZE 100

int main(int argc, char** argv) {

	FILE* file = NULL;

	char* command = NULL;
	char* fileChoice = NULL;
	char* newFilename = NULL;

	char* contents = NULL;

	// Raise error if user didn't input filename or compile mode
	if (!argv[1] || !argv[2]) {
		printf("[Error]: Some file input is missing"); exit(1);
	}

	newFilename = make_new_filename(argv[1], ".asm");	// Set output filename to be the code filename with a .asm extention
	
	// If user wants to compile in Hebrew, run the translator and replace the file with the translated file
	if (!strcmp(argv[2], "-h")) {

		/* 
		Allocate a string to run the python translator file
		The translator takes a source code and replaces Hebrew keywords with English keywords
		NOTE: Hebrew words that are not reserved get translated to a gibberish English word
		*/
		command = calloc(1, strlen("python translator.py ") + strlen(argv[1]) + 1);
		sprintf(command, "python translator.py %s", argv[1]);
		system(command);

		fileChoice = calloc(1, strlen(argv[1]) + strlen("translated_") + 1);
		sprintf(fileChoice, "translated_%s", argv[1]);

		free(command);
	}
	// Otherwise, we just want to use the normal filename
	else {

		fileChoice = calloc(1, strlen(argv[1]) + 1);
		strcpy(fileChoice, argv[1]);
	}

	if (!(file = fopen(fileChoice, "r"))) {		// Read file
		printf("File does not exist\n");	
		return 1;
	}
	
	contents = read_file(file);			// Read contents of file

	if (!contents) {
		printf("[Error]: Couldn't read file contents"); exit(1);
	}
	
	lexer_T* const lexer = init_lexer(contents);			// Initialize lexer
	parser_T* const parser = init_parser(lexer);			// Initialize Parser
	AST* const root = parser_parse(parser);					// Parse the tokens into an AST
	TAC_list* const instructions = traversal_visit(root);	// Visit the AST and generate an intermidiate representation

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
	if (!strcmp(argv[2], "-h")) {
		remove(fileChoice);
	}
		
	free(newFilename);
	free(fileChoice);
		
	return 0;
}

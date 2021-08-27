#include "TAC.h"
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
			/*while (lexer->index < lexer->contentsLength)
				printf("Token: %s\n", lexer_get_next_token(lexer)->value);*/
			parser = init_parser(lexer);
			root = parser_parse(parser);
			instructions = traversal_visit(root);

			for (i = 0; i < instructions->size; i++)
			{
				if (instructions->instructions[i]->op)
					printf("Operation: %s, ", typeToString(instructions->instructions[i]->op));
				printf("Arg1: %s (%p), ", (char*)instructions->instructions[i]->arg1, (TAC*)instructions->instructions[i]->arg1);
				if ((char*)instructions->instructions[i]->arg2)
					printf("Arg2: %s (%p), ", (char*)instructions->instructions[i]->arg2, (TAC*)instructions->instructions[i]->arg2);
				printf("Address: %p\n", (char*)instructions->instructions[i]);
			}

			free(contents);
		}
	}
	else
		printf("File does not exist\n");
	
	return 0;
}
#include "parser.h"
#define SIZE 100

int main(int argc, char** argv)
{
	FILE* file = NULL;
	lexer_T* lexer = NULL;
	token_T* token = NULL;
	parser_T* parser = NULL;
	AST* root = NULL;
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

			free(contents);
		}
	}
	else
		printf("File does not exist\n");
	
	return 0;
}
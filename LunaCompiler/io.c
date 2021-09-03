#include "io.h"


char* read_file(FILE* file)
{
	char* contents = NULL;
    char ch = 0;
    size_t i = 0;

    contents = calloc(1, sizeof(char));

    while ((ch = fgetc(file)) != EOF)
    {
        contents = realloc(contents, ++i);
        contents[i - 1] = ch;
    } 

    contents = realloc(contents, ++i);
    contents[i - 1] = '\0';       

    printf("contents:%s\n", contents);

    fclose(file);

    return contents;
}

/*
Function will perform the fgets command and also remove the newline
that might be at the end of the string - a known issue with fgets.
input: the buffer to read into, the number of chars to read
*/
void myFgets(char str[], int n)
{
    fgets(str, n, stdin);
    str[strcspn(str, "\n")] = 0;
}

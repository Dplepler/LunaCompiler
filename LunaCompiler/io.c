#include "io.h"

char* read_file(FILE* file)
{
	char* contents = NULL;
    unsigned int length = 0;
    unsigned int i = 0;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file);

    contents = (char*)calloc(1, length + 1);

    for (i = 0; i < length; i++)
        contents[i] = fgetc(file);

    contents[length] = 0;

    printf("contents: %s\n", contents);

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
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

char* make_new_filename(char* name)
{
    char* newFilename = calloc(1, sizeof(char));
    const char* fileExtention = ".asm";

    unsigned int i = 0;
    size_t size = 0;

    for (i = 0; i < strlen(name); i++)
    {
        if (name[i] != '.')
        {
            newFilename = realloc(newFilename, ++size);
            newFilename[size - 1] = name[i];
        }
        else
        {
            newFilename = realloc(newFilename, size + 1);
            newFilename[size] = '\0';

            newFilename = realloc(newFilename, size + strlen(fileExtention));
            strcat(newFilename, fileExtention);
            break;
        }
    }
        
    return newFilename;
}

size_t numOfDigits(int num)
{
    size_t counter = 0;
    do
    {
        num /= 10;
        counter++;

    } while (num > 0);
    
    return counter;
}

/*
isNum checks if string only contains numbers
Input: String to check
Output: True if string only contains digits, otherwise false
*/
bool isNum(char* value)
{
    bool flag = true;

    unsigned int i = 0;
    size_t size = strlen(value);


    for (i = 0; i < size && flag; i++)
    {
        if (!isdigit(value[i]))
            flag = false;
    }

    return flag;
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

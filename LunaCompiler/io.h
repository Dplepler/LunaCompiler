#ifndef IO_H
#define IO_H
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

char* read_file(FILE* file);
char* make_new_filename(char* name, char* extention);

bool isNum(char* value);

size_t numOfDigits(unsigned long num);

void assemble_file(char* filename);
void myFgets(char str[], int n);


#endif
 
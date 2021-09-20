#ifndef IO_H
#define IO_H
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>


char* read_file(FILE* file);
char* make_new_filename(char* name);

bool isNum(char* value);

void myFgets(char str[], int n);

size_t numOfDigits(int num);


#endif

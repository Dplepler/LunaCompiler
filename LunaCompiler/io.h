#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


static inline void* mcalloc(size_t count, size_t size) {

  void* ptr = calloc(count, size);

  if (!ptr) { printf("Heap error"); exit(1); }

  return ptr;
}

static inline void* mrealloc(void* ptr, size_t size) {

  void* tmp = realloc(ptr, size);
  return tmp ? tmp : ptr;
}

char* read_file(FILE* file);
char* make_new_filename(char* name, char* extention);

bool isNum(char* value);

size_t numOfDigits(long num);

void assemble_file(char* filename);
void myFgets(char str[], int n);


#endif
 
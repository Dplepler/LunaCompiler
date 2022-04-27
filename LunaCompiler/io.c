#include "io.h"

/*
read_file takes a file and returns it's contents in a string
Input: File
Output: Contents of the file
*/
char* read_file(FILE* file) {

  char* contents = NULL;
  char ch = 0;
  size_t i = 0;

  contents = mcalloc(1, sizeof(char));

  // While end of file is not reached, advance file string data and put the current char in it
  while ((ch = fgetc(file)) != EOF) {
  contents = mrealloc(contents, ++i);
  contents[i - 1] = ch;
  } 

  // Terminate string with a 0
  contents = mrealloc(contents, ++i);
  contents[i - 1] = '\0';     

  fclose(file);

  return contents;
}

const char* get_filename_ext(const char* name) {
  const char* dot = strrchr(name, '.');
  if (!dot || dot == name) return "";
  return dot + 1;
}

/*
make_new_filename gets a file and changes it's extention to the desired one
Input: Filename, desired new extention
Output: The same filename with a the desired extention
*/
char* make_new_filename(const char* name, const char* extention) {

  char* newFilename = mcalloc(1, sizeof(char));

  unsigned int i = 0;
  size_t size = 0;
  size_t length = strlen(name);

  for (i = 0; i < length; i++) {

    // For every normal character, we copy it as is
    if (name[i] != '.') {
      newFilename = mrealloc(newFilename, ++size);
      newFilename[size - 1] = name[i];
    }
    // When extention reached we copy the .asm extention instead
    else {
      // Before using strcat, we have to terminate the current string
      newFilename = mrealloc(newFilename, ++size);
      newFilename[size - 1] = '\0';

      size += strlen(extention) + 1;

      newFilename = mrealloc(newFilename, size);
      strcat(newFilename, extention);
      break;
    }
  }
    
  return newFilename;
}

/*
numOfDigits returns the amount of digits in a number
Input: Number
Output: Amount of digits
*/
size_t numOfDigits(long num) {

  size_t counter = 0;

  do {

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
bool isNum(char* value) {

  bool flag = true;
  size_t size = strlen(value);

  // Loop through the string, if we find 1 character that is not a num we can return false
  for (unsigned int i = 0; i < size && (flag = isdigit(value[i])); i++) { }

  return flag;
}

void assemble_file(char* filename) {

  char* command = NULL;
  char* objectFilename = make_new_filename(filename, ".obj");
  char* exeFilename = make_new_filename(filename, ".exe");

  command = mcalloc(1, strlen("C:\\masm32\\bin\\ml /c /Zd /coff %s") + strlen(filename) + 1);
  sprintf(command, "C:\\masm32\\bin\\ml /c /Zd /coff %s", filename);
  system(command);

  free(command);

  command = mcalloc(1, strlen("C:\\masm32\\bin\\Link /SUBSYSTEM:CONSOLE %s") + strlen(objectFilename) + 1);
  sprintf(command, "C:\\masm32\\bin\\Link /SUBSYSTEM:CONSOLE %s", objectFilename);
  system(command);

  free(command);
  //system("cls");
  system(exeFilename);

  free(objectFilename);
  free(exeFilename);
}


/*
myFgets will perform the fgets command and also remove the newline
that might be at the end of the string - a known issue with fgets.
input: the buffer to read into, the number of chars to read
*/
void myFgets(char str[], int n) {

  fgets(str, n, stdin);
  str[strcspn(str, "\n")] = 0;
}
 
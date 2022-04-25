#ifndef AST_H
#define AST_H
#include "tokens.h"
#include "symbolTable.h"

typedef struct AST_STRUCT {

  enum AST_TYPE_ENUM {

    AST_PROGRAM = 24,    // Continue enumeration from tokens (tokens.c)
    AST_FUNCTION,
    AST_DEF_AMOUNT,
    AST_COMPOUND,
    AST_INT,
    AST_STRING,
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_ASSIGNMENT,
    AST_VARIABLE_DEC,
    AST_VARIABLE,
    AST_FUNC_CALL,
    AST_PARAM,
    AST_IF,
    AST_IFZ,
    AST_GOTO,
    AST_LABEL,
    AST_LOOP_LABEL,
    AST_WHILE,
    AST_COMPARE,
    AST_PRINT,
    AST_RETURN,

  } type;

  // Compound
  struct AST_STRUCT** children;

  // Branch
  struct AST_STRUCT* condition;
  struct AST_STRUCT* if_body;
  struct AST_STRUCT* else_body;
  int type_c;              // Type of condition (>, ==, <=)


  // Variable declaration
  dtype var_type;
  
  // Functions
  struct AST_STRUCT** function_list;
  struct AST_STRUCT** function_def_args;
  struct AST_STRUCT* function_body;

  size_t functionsSize;


  // Name used for different types of nodes, (e.g a variable node would have it's node->name be the identifier)
  char* name;

  // Same for size
  size_t size;

  // Int node
  char* int_value;

  // Function call
  struct AST_STRUCT** arguments;


  struct AST_STRUCT* leftChild;
  struct AST_STRUCT* value;
  struct AST_STRUCT* rightChild;

} AST;

AST* init_AST(int type);
AST* AST_initChildren(AST* left, AST* right, int type);
char* typeToString(int type);
void AST_free_AST(AST* node);

#endif
 
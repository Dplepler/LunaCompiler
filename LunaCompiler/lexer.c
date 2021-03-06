#include "lexer.h"

/*
init_lexer initializes the lexer with the source code (contents)
Input: Source code
Output: Lexer
*/
lexer_T* init_lexer(char* contents) {

  lexer_T* lexer = mcalloc(1, sizeof(lexer_T));

  lexer->contents = contents;
  lexer->contentsLength = strlen(contents);
  lexer->c = contents[lexer->index];

  lexer->lineIndex = 1;

  lexer->tokens = mcalloc(1, sizeof(token_T*));

  return lexer;
}

/*
lexer_advance advances the lexer by 1
Input: lexer
Output: None
*/
void lexer_advance(lexer_T* lexer) {  
  lexer->c = lexer->index < lexer->contentsLength ? lexer->contents[++lexer->index] : '\0';
}

/*
lexer_peek returns the file character from the current index position of the lexer + the offset the function is given
Input: Lexer, offset to add to current index
Output: File character in the desired position
*/
char lexer_peek(lexer_T* lexer, size_t offset) {

  char lexeme = -1;

  if (lexer->index + 1 < lexer->contentsLength) {
    lexeme = lexer->contents[lexer->index + offset];
  }

  return lexeme;
}

/*
lexer_token_peeks returns the next token without advancing
Input: Lexer
Output: Next token
*/
token_T* lexer_token_peek(lexer_T* lexer, unsigned int offset) {

  token_T* token = NULL;

  size_t saveLoc = lexer->index;
  size_t saveLine = lexer->lineIndex;

  for (unsigned int i = 0; i < offset; i++) {
    token = lexer_get_next_token(lexer);
  }

  lexer->index = saveLoc;    // Return previous index
  lexer->lineIndex = saveLine;
  lexer->c = lexer->contents[lexer->index];

  return token;
}

/*
lexer_advance_current initializes a token with a type and a value
Input: Lexer, type of token
Output: Initialized token
*/
token_T* lexer_advance_current(lexer_T* lexer, int type) {

  token_T* token = NULL;
  char* value = mcalloc(1, VALUE_SIZE);
  
  value[0] = lexer->c;

  // If token is 2 characters long (e.g: <=, ==)
  if (type == TOKEN_ELESS || type == TOKEN_EMORE || type == TOKEN_DEQUAL || type == TOKEN_NEQUAL) {
    
    value = mrealloc(value, VALUE_SIZE + 1);
    lexer_advance(lexer);
    value[1] = lexer->c;
    value[2] = '\0';
  }
  else  {
    value[1] = '\0';
  }
    
  token = init_token(type, value);
  lexer_advance(lexer);

  return token;
}

/*
lexer_skip_whitespace Skips all the unnecessary spaces and white space
Input: Lexer
Output: None
*/
void lexer_skip_whitespace(lexer_T* lexer) {

  // Skipping Whitespace
  while (lexer->c == ' ' || lexer->c == '\n' || lexer->c == '\t') {
    if (lexer->c == '\n') {
      lexer->lineIndex++;
    }
      
    lexer_advance(lexer);
  }

  lexer_skip_comments(lexer);
}

void lexer_skip_comments(lexer_T* lexer) {

  // Skipping comments
  if (lexer->c == '~') {

    while (lexer->c != '\n') {

      if (lexer->c == '\0') {
        break;
      }
        
      lexer_advance(lexer);
    }
      
    lexer_advance(lexer);    // Skipping newline

    lexer->lineIndex++;      // Advance newline index

    lexer_skip_whitespace(lexer);
  }
}

/*
lexer_get_next_token gets the next token and initializes a value and type
Input: Lexer
Output: The new token
*/
token_T* lexer_get_next_token(lexer_T* lexer) {

  token_T* token = NULL;

  lexer_skip_whitespace(lexer);

  if (isalpha(lexer->c))      { token = lexer_collect_id(lexer); }
  else if (isdigit(lexer->c)) { token = lexer_collect_number(lexer); }
  else if (lexer->c == '#')   { token = lexer_asm(lexer); }
  else {

    switch (lexer->c) {

      case '!': token = lexer_peek(lexer, 1) == '=' ? lexer_advance_current(lexer, TOKEN_NEQUAL)
        : lexer_advance_current(lexer, TOKEN_NOT); break;

      case '=': token = lexer_peek(lexer, 1) == '=' ? lexer_advance_current(lexer, TOKEN_DEQUAL) 
        : lexer_advance_current(lexer, TOKEN_EQUALS); break;

      case '<': token = lexer_peek(lexer, 1) == '=' ? lexer_advance_current(lexer, TOKEN_ELESS)
        : lexer_advance_current(lexer, TOKEN_LESS); break;

      case '>': token = lexer_peek(lexer, 1) == '=' ? lexer_advance_current(lexer, TOKEN_EMORE)
        : lexer_advance_current(lexer, TOKEN_MORE); break;

      case '"': token = lexer_collect_string(lexer);                break;
      case ';': token = lexer_advance_current(lexer, TOKEN_SEMI);   break;
      case '(': token = lexer_advance_current(lexer, TOKEN_LPAREN); break;
      case ')': token = lexer_advance_current(lexer, TOKEN_RPAREN); break;
      case '*': token = lexer_advance_current(lexer, TOKEN_MUL);    break;
      case '+': token = lexer_advance_current(lexer, TOKEN_ADD);    break;
      case '/': token = lexer_advance_current(lexer, TOKEN_DIV);    break;
      case '-': token = lexer_advance_current(lexer, TOKEN_SUB);    break;
      case ',': token = lexer_advance_current(lexer, TOKEN_COMMA);  break;
      case '{': token = lexer_advance_current(lexer, TOKEN_LBRACE); break;
      case '}': token = lexer_advance_current(lexer, TOKEN_RBRACE); break;
      case '\0': token = lexer_advance_current(lexer, TOKEN_EOF);   break;
      default: printf("[Error in line %zu]: Unknown lexeme: '%c'", lexer->lineIndex, lexer->c); exit(1);
    }
  }
  
  lexer_token_list_push(lexer, token);  // Push token
  return token;
}

/*
lexer_collect_id collects an identifier token
Input: Lexer
Output: Identifier token
*/
token_T* lexer_collect_id(lexer_T* lexer) {

  char* id = mcalloc(1, sizeof(char));
  size_t size = 0;
  
  /* An ID has to start with a letter but can contain numbers, letters and underscore */
  while (isalpha(lexer->c) || isdigit(lexer->c) || lexer->c == '_') {

    id = mrealloc(id, ++size);
    id[size - 1] = lexer->c;
    lexer_advance(lexer);
  }

  // Terminate the string with a 0
  id = mrealloc(id, size + 1);
  id[size] = '\0';

  return init_token(TOKEN_ID, id);
}
 
/*
lexer_collect_number collects a number token
Input: Lexer
Output: Number token
*/
token_T* lexer_collect_number(lexer_T* lexer) {

  char* num = NULL;
  size_t size = 0;

  /* Collect number */
  while (isdigit(lexer->c)) {
    num = mrealloc(num, ++size);
    num[size - 1] = lexer->c;
    lexer_advance(lexer);
  }
  /* Terminate string with a 0 */
  num = mrealloc(num, size + 1);
  num[size] = '\0';

  return init_token(TOKEN_NUMBER, num);
}

token_T* lexer_asm(lexer_T* lexer) {

  char* code = NULL;
  size_t size = 0;

  lexer_advance(lexer);
  while (lexer->c == '\t') { lexer_advance(lexer); }
  
  while (lexer->c != '#') {
    code = mrealloc(code, ++size);
    code[size - 1] = lexer->c;
    lexer_advance(lexer);
    while (lexer->c == '\t') { lexer_advance(lexer); }
  }

  lexer_advance(lexer);

  /* Terminate string with a 0 */
  code = mrealloc(code, size + 1);
  code[size] = '\0';

  return init_token(TOKEN_ASM, code);
}

/*
lexer_collect_string collects a string token
Input: Lexer
Output: String token
*/
token_T* lexer_collect_string(lexer_T* lexer) {

  char* string = mcalloc(1, sizeof(char));
  size_t size = 0;

  size_t line = lexer->lineIndex;

  lexer_advance(lexer);

  /* Collect string */
  while (lexer->c != '"') {

    // If we reached the end of the file without getting an ending quote, raise error
    if (lexer->c == '\0') {

      printf("[Error in line %zu]: Start of string was never ended", line);
      exit(1);
    }

    string = mrealloc(string, ++size);
    string[size - 1] = lexer->c;  
    lexer_advance(lexer);
  }
  // Terminate string with a 0
  string = mrealloc(string, size + 1);
  string[size] = '\0';

  lexer_advance(lexer);

  return init_token(TOKEN_STRING, string);
}

/*
lexer_token_list_push pushes a token to the list of tokens so we can free them later
Input: Lexer, token to push
Output: None
*/
void lexer_token_list_push(lexer_T* lexer, token_T* token) {

  lexer->tokens = mrealloc(lexer->tokens, sizeof(token_T*) * ++lexer->tokensSize);
  lexer->tokens[lexer->tokensSize - 1] = token;
}

/*
lexer_free_tokens frees the list of tokens and their values
Input: Lexer with the list of tokens
Output: None
*/
void lexer_free_tokens(lexer_T* lexer) {

  for (unsigned int i = 0; i < lexer->tokensSize; i++) {
    free(lexer->tokens[i]->value);
    free(lexer->tokens[i]);
  }

  free(lexer->tokens);
}
 
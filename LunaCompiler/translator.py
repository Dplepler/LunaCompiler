import sys
import enum

hebrew_alphabet = "אבגדהוזחטיכלמנסעפצקרשתםןץףך"
english_alphabet = "abcdefghijklmnopqrstuvxwyz"

reserved_words = ["תדפיס", "אם", "אחרת", "בזמןש", "מספר", "תחזיר"]

class Lexer(enum.Enum):
    data = 0
    index = 1
    token = 2
    file = 3

def collect_id(lexer):

    lexer[Lexer.token.value] = []

    # Collect an id until there are no more hebrew letters
    while lexer[Lexer.data.value][lexer[Lexer.index.value]] in hebrew_alphabet:
        lexer[Lexer.token.value].append(lexer[Lexer.data.value][lexer[Lexer.index.value]])
        lexer[Lexer.index.value] += 1
    
    # If id is a reserved word replace it with the English equivalent
    if "".join(lexer[Lexer.token.value]) in reserved_words:
        lexer[Lexer.file.value].write(replace_keyword("".join(lexer[Lexer.token.value])))
    # If it's a variable or function name, we want to just replace it with random English letters
    else:
        lexer[Lexer.file.value].write(create_id(lexer[Lexer.token.value]))

def replace_keyword(token):

    # Reserved words
    if token == "תדפיס":
        return "print"
    elif token == "אם":
        return "if"
    elif token == "אחרת":
        return "else"
    elif token == "בזמןש":
        return "while"
    elif token == "מספר":
        return "int"
    elif token == "תחזיר":
        return "return"

def create_id(token):

    hebrew_index = 0

    for index, value in enumerate(token):

        hebrew_index = hebrew_alphabet.index(value)

        # In Hebrew there are letters that only exist in the end of words, we can translate them to 
        # their regular representation so that we can translate all letters to English
        if  value == 'ם':
            token[index] = 'm'
        elif value == 'ן':
            token[index] = 'n'
        elif value == 'ץ':
            token[index] = 'r'
        elif value == 'ף':
            token[index] = 'q'
        elif token[index] == 'ך':
            token[index] = 'k'
        else:
            token[index] = english_alphabet[hebrew_alphabet.index(value)]

    return "".join(token)

def translator():

    try:
        file = open(sys.argv[1], 'r', encoding='utf-8')
    except:
        sys.exit("[ERROR]: File was not found")

    translated_file = open("translated_" + sys.argv[1], 'w', encoding='utf-8')
    data = file.read()
    i = 0
    token = []

    lexer = [data, i, token, translated_file]
    length = len(data)

    while (lexer[Lexer.index.value] < length):
        if lexer[Lexer.data.value][lexer[Lexer.index.value]] in hebrew_alphabet:
            collect_id(lexer)
        elif lexer[Lexer.data.value][lexer[Lexer.index.value]] in english_alphabet:
            sys.exit("[ERROR]: Cannot write in English when using Hebrew mode")
        else:
            # Skipping comments
            if lexer[Lexer.data.value][lexer[Lexer.index.value]] == '~':
                while lexer[Lexer.data.value][lexer[Lexer.index.value]] != '\n':
                    lexer[Lexer.index.value] += 1
            # If character is not in Hebrew, implement it as is
            lexer[Lexer.file.value].write(lexer[Lexer.data.value][lexer[Lexer.index.value]])
            lexer[Lexer.index.value] += 1

    translated_file.close()
    file.close()


def main():
 
    if len(sys.argv) < 2:
        sys.exit("[ERROR]: Not enough arguments")

    translator()

if __name__ == "__main__":
    main()




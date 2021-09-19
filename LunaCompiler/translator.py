import sys
import enum

hebrew_alphabet = "אבגדהוזחטיכלמנסעפצקרשת"
english_alphabet = "abcdefghijklmnopqrstuvxwyz"

reserved_words = ["תדפיס", "אם", "אחרת", "בזמן ש", "מספר", "תחזיר"]

class Lexer(enum.Enum):
    data = 0
    index = 1
    token = 2
    file = 3

def collect_id(lexer):

    
    while lexer[Lexer.data.value][lexer[Lexer.index.value]] in hebrew_alphabet:
        lexer[Lexer.token.value].append(lexer[Lexer.data.value][lexer[Lexer.index.value]])
        lexer[Lexer.index.value] += 1

    if "".join(lexer[Lexer.token.value]) in reserved_words:
        lexer[Lexer.file.value].write(replace_keyword("".join(lexer[Lexer.token.value])))
    else:
        lexer[Lexer.file.value].write(create_id(lexer[Lexer.token.value]))

def replace_keyword(token):

    if token == "תדפיס":
        return "print"
    elif token == "אם":
        return "if"
    elif token == "אחרת":
        return "else"
    elif token == "בזמן ש":
        return "while"
    elif token == "מספר":
        return "int"
    elif token == "תחזיר":
        return "return"

def create_id(token):

    print(token)
    for index, value in enumerate(token):
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
            lexer[Lexer.index.value] += 1
            lexer[Lexer.file.value].write(lexer[Lexer.data.value][i])

    translated_file.close()
    file.close()


def main():
 
    if len(sys.argv) < 2:
        sys.exit("[ERROR]: Not enough arguments")

    translator()

if __name__ == "__main__":
    main()




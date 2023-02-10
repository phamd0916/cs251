#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"
#include "linkedlist.h"
#include "talloc.h"

int isDigit(char input) {
    if(input == '0' || input == '1' || input == '2' || input == '3' || input == '4' ||
    input == '5' || input == '6' || input == '7' || input == '8' || input == '9') {
        return 1; 
    } 
    else {
        return 0;
    }
}

int isSign(char input) {
    if( input == '+' || input == '-'){
        return 1;
    } 
    else {
        return 0;
    }
}

int isUinteger(char *input) {
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isDigit(input[i])) {
            return 0;
        } 
    }
    return 1;
}

int isUdecimal(char *input) {
    int decimals = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (!isDigit(input[i])) {
            if (input[i] == '.') {
                if (decimals == 1) {
                    return 0;
                }
                decimals = decimals + 1;
            }
            else {
                return 0;
            }
        }    
    }
    return 1;
}

int isUreal(char *input) {
    if(isUinteger(input) == 1) {
        return 1;
    }
    else if (isUdecimal(input) == 1) {
        return 1;
    }
    else {
        return 0;
    }
}

int isNumber(char *input) {
    int size = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        size++;
    }
    if (isSign(input[0]) == 1 && size > 1 && isUreal(&input[1]) == 1) {
        return 1;
    }
    else if (isUreal(input) == 1) {
        return 1;
    }
    else {
        return 0;
    }
    
}

int isLetter(char input) {
    if(input == 'a' || input == 'b' || input == 'c' || input == 'd' || input == 'e' || input == 'f'
    || input == 'g' || input == 'h' || input == 'i' || input == 'j' || input == 'k' || input == 'l'
    || input == 'm' || input == 'n' || input == 'o' || input == 'p' || input == 'q' || input == 'r'
    || input == 's' || input == 't' || input == 'u' || input == 'v' || input == 'w' || input == 'x'
    || input == 'y' || input == 'z' || input == 'A' || input == 'B' || input == 'C' || input == 'D'
    || input == 'E' || input == 'F' || input == 'G' || input == 'H' || input == 'I' || input == 'J'
    || input == 'K' || input == 'L' || input == 'M' || input == 'N' || input == 'O' || input == 'P'
    || input == 'Q' || input == 'R' || input == 'S' || input == 'T' || input == 'U' || input == 'V'
    || input == 'W' || input == 'X' || input == 'Y' || input == 'Z') {
        return 1;
    } 
    else {
        return 0;
    }
}

int isInitial(char input) {
    if(input == '!' || input == '$' || input == '%' || input == '&' || input == '*' || input == '/'
    || input == ':' || input == '>' || input == '<' || input == '=' || input == '?' || input == '~'
    || input == '_' || input == '^') {
        return 1;
    } 
    else if(isLetter(input) == 1) {
        return 1;
    }
    else {
        return 0;
    }
}

int isSubsequent(char input) {
    if (isInitial(input) == 1 || isDigit(input) == 1) {
        return 1;
    }
    else if (input == '.' || input == '+' || input == '-') {
        return 1;
    }
    else {
        return 0;
    }
}

int isIdentifier(char *input) {
    if (isInitial(input[0]) == 1) {
        for (int i = 0; input[i] != '\0'; i++) {
            if (!isSubsequent(input[i])) {
                return 0;
            } 
        }
        return 1;
    }
    else if (input[0] == '+' && input[1] == '\0') {
        return 1;
    }
    else if (input[0] == '-' && input[1] == '\0') {
        return 1;
    }
    else {
        return 0;
    }
}

int isBoolean(char *input) {
    int size = 0;
    for(int i = 0; input[i] != '\0'; i++) {
        size++;
    }
    if(size > 2 || input[0] != '#') {
        return 0;
    }
    else if (input[1] == 'f' || input[1] == 't') {
        return 1;
    } else {
        return 0;
    }
}

int isParan(char input) {
    if (input == '(' || input == ')') {
        return 1;
    }
    return 0;
}

int isSpace(char input) {
    if (input == ' ') {
        return 1;
    }
    else if (input == '\n') {
        return 1;
    }
    return 0;
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF) {
        
        if (charRead == '(') {
            char *str_value = "(";
            Value *open =  talloc(sizeof(Value));
            open->type = OPEN_TYPE;
            open->s = str_value;
            list = cons(open, list);

        } else if (charRead == ')') {
            char *str_value = ")";
            Value *close = talloc(sizeof(Value));
            close->type = CLOSE_TYPE;
            close->s = str_value;
            list = cons(close, list);

        // check blanks
        } else if (isSpace(charRead) || charRead == '\n') {
            ;

        // check strings
        } else if (charRead == '"') {
            char *test_string = talloc(sizeof(char) * 300);
            test_string[0] = '\0';
            int length = 0; //used to keep track of length of array
            int memSize = 299;
            test_string[length] = '\0';
            charRead = fgetc(stdin);

            while (charRead != '"') {
                if (charRead == EOF) {
                    printf("String untokenizable, missing quote\n");
                    texit(1);
                }
                // If an escape character is encountered in string,
                // function says charRead is intended character
                else if (charRead == '\\'){
                    char nextChar = fgetc(stdin);
                    if (nextChar == 'n') {
                        charRead = '\n';
                    }
                    else if (nextChar == 't') {
                        charRead = '\t';
                    }
                    else if (nextChar == '\'') {
                        charRead = '\'';
                    }
                    else if (nextChar == '\"') {
                        charRead = '\"';
                    }

                    if (memSize == length) {
                        char *temp = talloc(sizeof(char) * ((memSize * 2) + 1));
                        for(int i = 0; test_string[i] != '\0'; i++) {
                            temp[i] = test_string[i];
                        }
                        memSize = memSize * 2;
                        test_string = temp;
                    }
                    test_string[length] = charRead;
                    length++;
                    test_string[length] = '\0';
                    
                }
                else {
            
                    // Adds character to test_string
                    if (memSize == length) {
                        char *temp = talloc(sizeof(char) * ((memSize * 2) + 1));
                        for(int i = 0; test_string[i] != '\0'; i++) {
                            temp[i] = test_string[i];
                        }
                        memSize = memSize * 2;
                        test_string = temp;
                    }
                    test_string[length] = charRead;
                    length++;
                    // Adds null terminator for helper function use
                    test_string[length] = '\0';
                    
                }
                charRead = fgetc(stdin);
            }
            Value *str = talloc(sizeof(Value));
            str->type = STR_TYPE;
            str->s = test_string;
            list = cons(str, list);
        }

        //check comments
        else if (charRead ==';' && charRead != EOF){
            while(charRead != '\n'){
                charRead = (char)fgetc(stdin);
            }
            ;
        }
        
        // check number, symbols and booleans
        else {
            char *token = talloc(sizeof(char) * 300);
            token[0] = '\0';
            int length2 = 0;
            int memSize2 = 299;
            while ((!isParan(charRead)) && (!isSpace(charRead))) {
                if (charRead == EOF) {
                    printf("Syntax Error: Incomplete Token\n");
                    texit(1);
                }
                else if (charRead == '\n') {
                    ;
                }
                else {
                    // Adds the valid character 
                    if (length2 == memSize2) {
                        char *temp2 = talloc(sizeof(char) * ((memSize2 * 2) + 1));
                        for(int i= 0; token[i] != '\0'; i++) {
                            temp2[i] = token[i];
                        }
                        token = temp2;
                        memSize2 = memSize2 * 2;
                    }
                    token[length2] = charRead;
                    length2++;
                    // Null terminator used for helper functions
                    token[length2] = '\0';
                }
                charRead = fgetc(stdin);
            }

            // Ungetting a character that was a brace or a space
            ungetc(charRead, stdin);

            if (isNumber(token)) {
                // Determine if integer or float, store in value...
                if (isSign(token[0])) {
                    char *utoken = talloc(sizeof(char) * strlen(token) - 1);
                    for (int i = 1; token[i] != '\0'; i++) {
                        utoken[i-1] = token[i];
                    }
                    int index = strlen(token) - 2;
                    utoken[index] = '\0';
                    // If the token is a signed int of decimal
                    if (isUinteger(utoken)) {
                        Value *int_value = talloc(sizeof(Value));
                        int_value->type = INT_TYPE;
                        int_value->i = atoi(token);
                        list = cons(int_value, list);
                    }
                    else if (isUdecimal(utoken)) {
                        Value *double_value = talloc(sizeof(Value));
                        double_value->type = DOUBLE_TYPE;
                        double_value->d = atof(token);
                        list = cons(double_value, list);
                    }
                    // If the number isn't an int or decimal, throw error
                    else {
                        printf("Syntax Error: '%s' untokenizable \n", token);
                    }
                }
                // If statements where the first character isn't a sign
                else if (!isSign(token[0])) {
                    if (isUinteger(token)) {
                        Value *int_value = talloc(sizeof(Value));
                        int_value->type = INT_TYPE;
                        int_value->i = atoi(token);
                        list = cons(int_value, list);
                    }
                    else if (isUdecimal(token)) {
                        Value *double_value = talloc(sizeof(Value));
                        double_value->type = DOUBLE_TYPE;
                        double_value->d = atof(token);
                        list = cons(double_value, list);
                    }
                    else {
                        printf("Syntax Error: '%s' untokenizable \n", token);
                    }
                }
                // If the first character somehow doesn't fit into first two ifs
                else {
                    printf("Syntax Error: '%s' untokenizable \n", token);
                    texit(1);
                }
                    
            }

            //check symbol
            else if(isIdentifier(token)) {
                Value *symbol = talloc(sizeof(Value));
                symbol->type = SYMBOL_TYPE;
                symbol->s = token;
                list = cons(symbol, list);
            }

            //check boolean
            else if (isBoolean(token)) {
                if (token[1] == 'f') {
                    Value *bool_value = talloc(sizeof(Value));
                    bool_value->type = BOOL_TYPE;
                    bool_value->i = 0;
                    list = cons(bool_value, list);
                }
                else {
                    Value *bool_value = talloc(sizeof(Value));
                    bool_value->type = BOOL_TYPE;
                    bool_value->i = 1;
                    list = cons(bool_value, list);
                }
            }

            //untokenizable
            else {
                printf("Syntax error\n");
                texit(1);
            }
        }
        charRead = (char)fgetc(stdin);
    }

    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {
    Value *current = list;
    while (!isNull(current)) {
        switch (car(current)->type) {
            case INT_TYPE:
                printf("%i:integer\n", car(current)->i);
                break;
            case DOUBLE_TYPE:
                printf("%f:double\n", car(current)->d);
                break;
            case STR_TYPE:
                printf("\"%s\":string\n", car(current)->s);
                break;
            case BOOL_TYPE:
                if (car(current)->i == 0) {
                    printf("#f:boolean\n");
                }
                else {
                    printf("#t:boolean\n");
                }
                break;
            case SYMBOL_TYPE:
                printf("%s:symbol\n", car(current)->s);
                break;
            case OPEN_TYPE:
                printf("(:open\n");
                break;
            case CLOSE_TYPE:
                printf("):close\n");
                break;
            default:
                break;
        }
        current = cdr(current);
    }
}

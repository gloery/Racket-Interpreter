//Code written by Emily Johnston, Charlotte Foran, and Gordon Loery

#include "tokenizer.h"
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"
#include <stdio.h>
#include <string.h>

//declare functions here since we are not allowed
//to edit the header files
int isDigit(char);
int isInitial(char);
int isLetter(char);
int isSubsequent(char);
int isWhitespace(char);


// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = fgetc(stdin);
    
    //create buffer for temporarily storing strings
    int bufferSize = 50;
    char *bufferArray = talloc(sizeof(char)*bufferSize);

    while (charRead != EOF) {
        //open paren
        if (charRead == '('){
            Value *node = talloc(sizeof(Value));
            node->type = OPEN_TYPE;
            list = cons(node, list);
        } 
        //closed paren
        else if (charRead == ')') {
            Value *node = talloc(sizeof(Value));
            node->type = CLOSE_TYPE;
            list = cons(node, list);
        } 
        //whitespace
        else if (isWhitespace(charRead)) {
            /*continue without doing anything*/
        }
        // string
        else if (charRead == '\"') {
            int count = 0;
            charRead = fgetc(stdin);
            // While we have not reached the end of the string, add chars to the buffer
            while (charRead != '\"'){
                // If the file ends before the end of the string, error
                if(charRead == EOF){
                    printf("Syntax error: encountered EOF in middle of string\n");
                    texit(EXIT_FAILURE);
                }
                // if string is too long for storage buffer, make the buffer bigger
                if (count + 2 >= bufferSize){
                    bufferSize += 50;
                    char *temp = talloc(sizeof(char)*bufferSize);
                    strcpy(temp,bufferArray);
                    bufferArray = temp;
                }
                char nextChar = fgetc(stdin);
                // If the next character is a quote and cur char is a backslash, 
                // it's an escaped quote, not the end of the string
                if(nextChar == '\"' && charRead == '\\'){
                        bufferArray[count] = charRead;
                        bufferArray[count + 1] = nextChar;
                        count++;
                        
                        charRead = fgetc(stdin);
                } else {
                    bufferArray[count] = charRead;
                    charRead = nextChar;
                }
                count++;
            }
            // We have reached the end of the string, null terminate it 
            // and copy into a new char array
            bufferArray[count] = '\0';
            count++;
            char *finalStr = talloc(sizeof(char)*count);
            strcpy(finalStr,bufferArray);
            
            // Create a new node, put the string in it, add to the linked list
            Value *node = talloc(sizeof(Value));
            node->type = STR_TYPE;
            node->s = finalStr;
            list = cons(node, list);

        } 
        // boolean
        else if (charRead == '#'){
            charRead = fgetc(stdin);
            //check to make sure t or f follows # sign
            if (charRead == 't' || charRead == 'f'){
                char nextChar = fgetc(stdin);
                //check to make sure there's nothing else besides t or f
                if (isWhitespace(nextChar) || nextChar == ')' || nextChar == '('){
                    //create node and add to list of tokens
                    Value *node = talloc(sizeof(Value));
                    node->type = BOOL_TYPE;
                    if(charRead == 't'){
                        node->s = "#t";
                    }
                    else{
                        node->s = "#f";
                    }
                    list = cons(node, list);
                    
                }
                else{
                    printf("Syntax error: not a boolean\n");
                    texit(EXIT_FAILURE);
                }
                ungetc(nextChar, stdin);
            } else {
                printf("Syntax error: not a boolean\n");
                texit(EXIT_FAILURE);
            }
            
        } 
 
        // number or + or -
        else if (isDigit(charRead) || charRead == '.' || charRead == '+' || charRead == '-'){
            //variable to keep track of number of periods
            int seenPeriod = 0;
            int count = 1;
            //add first thing to buffer (to check if it's + or - later)
            bufferArray[0] = charRead;
            charRead = fgetc(stdin);
            //while we haven't yet reached the end of the number
            while (!isWhitespace(charRead) && charRead != ')' && charRead != '(' && charRead != EOF){
                //check if next thing is digit or period
                if (isDigit(charRead) || charRead == '.'){
                    // if it's a period, make sure it's the only period
                    if (charRead == '.'){
                        if (seenPeriod){
                            printf("Syntax error: too many decimal points in the number\n");
                            texit(EXIT_FAILURE);
                        }
                        else {
                            seenPeriod = 1;
                        }
                    }
                    // if string too long for storage buffer, increase size
                    if (count >= bufferSize){
                        bufferSize += 50;
                        char *temp = talloc(sizeof(char)*bufferSize);
                        strcpy(temp,bufferArray);
                        bufferArray = temp;
                    }
                    bufferArray[count] = charRead;
                    count++;  
                }
                else {
                    printf("Syntax error: Not a Number\n");
                    texit(EXIT_FAILURE);
                }
                charRead = fgetc(stdin);
            }
            //we got one too many things, so put one back
            ungetc(charRead, stdin);
            bufferArray[count] = '\0';
            count++;
            //check if current string is only a + or - sign
            if (!(strcmp(bufferArray,"+")) || !(strcmp(bufferArray,"-"))){
                //if so, treat it like a symbol
                Value *node = talloc(sizeof(Value));
                node->type = SYMBOL_TYPE;
                char *symbol = talloc(sizeof(char) * 2);
                strcpy(symbol,bufferArray);
                node->s = symbol;
                list = cons(node, list);
            }
            else{
                //create a string of the correct size, and copy
                //the bufferArray we have into that string
                char *finalStr = talloc(sizeof(char)*count);
                strcpy(finalStr,bufferArray);
                Value *node = talloc(sizeof(Value));
                //change that string into a float or int, based on type
                if (seenPeriod){
                    float finalFloat;
                    finalFloat = atof(finalStr);
                    
                    node->type = DOUBLE_TYPE;
                    node->d = finalFloat;
                }
                else{
                    int finalInt;
                    finalInt = atoi(finalStr);
                    
                    node->type = INT_TYPE;
                    node->i = finalInt;
                }
                //add to token list
                list = cons(node, list);
            }
        }
        
        
        
        // symbol
        else if (isInitial(charRead)){
            int count = 0;
            //while next thing read is part of the symbol
            while (!isWhitespace(charRead) && charRead != ')' && charRead != '(' && charRead != EOF){
                //check next thing is a valid char for symbol type
                if (isSubsequent(charRead)){
                    // if string too long for storage buffer, increase size
                    if (count >= bufferSize){
                        bufferSize += 50;
                        char *temp = talloc(sizeof(char)*bufferSize);
                        strcpy(temp,bufferArray);
                        bufferArray = temp;
                    }
                    bufferArray[count] = charRead;
                    count++;
                }
                else {
                    printf("Syntax error: Not a valid symbol %c\n", charRead);
                    texit(EXIT_FAILURE);
                }
                charRead = fgetc(stdin);
            }
            ungetc(charRead, stdin);
            bufferArray[count] = '\0';
            count++;
            //put symbol in new array of proper size
            char *finalStr = talloc(sizeof(char)*count);
            strcpy(finalStr,bufferArray);
            
            //store it in token list
            Value *node = talloc(sizeof(Value));
            node->type = SYMBOL_TYPE;
            node->s = finalStr;
            list = cons(node, list);
            
            
        }
        //comment
        else if (charRead == ';'){
            while (charRead != '\n' && charRead != EOF){
                charRead = fgetc(stdin);
            }
        }
        // unrecognized character
        else {
            printf("Syntax error: Character %c unknown\n", charRead);
            texit(EXIT_FAILURE);  
        }

        charRead = fgetc(stdin);
        
    }
    //TODO: maybe add makeNull to end of list here???????
    Value *revList = reverse(list);
    return revList;
}

/*
check if the given character c is a digit
*/
int isDigit(char c){
    return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
        c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
}

/*
check if the given character c is a letter
*/
int isLetter(char c){
    return ((c > 64 && c < 91) || (c > 96 && c < 123));
}

/*
check if the given character c can begin a symbol
*/
int isInitial(char c){
    return (isLetter(c) || c == '!' || c == '$' || c == '%' || c == '&' || 
            c == '*' || c == '/' || c == ':' || c == '<' || c == '=' || 
            c == '>' || c == '?' || c == '~' || c == '_' || c == '^');
}

/*
check if the given character c is a valid non-starter symbol character
*/
int isSubsequent(char c){
    return (isInitial(c) || isDigit(c) || c == '.' || c == '+' || c == '-');
}

/*
check if the given character c is whitespace
*/
int isWhitespace(char c){
    return (c == '\n' || c == '\t' || c == ' ');
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    
        switch(list->type){
            case NULL_TYPE:
                printf("\n");
                break;
            case CONS_TYPE:
                displayTokens(car(list));
                displayTokens(cdr(list));
                break;
            case INT_TYPE:
                printf("%i : integer\n", list->i);
                break;
            case DOUBLE_TYPE:
                printf("%f : float\n", list->d);
                break;
            case STR_TYPE:
                printf("\"%s\" : string\n", list->s);
                break;
            case BOOL_TYPE:
                printf("%s : boolean\n", list->s);
                break;
            case OPEN_TYPE:
                printf("( : open\n");
                break;
            case CLOSE_TYPE:
                printf(") : close\n");
                break;
            case SYMBOL_TYPE:
                printf("%s : symbol\n", list->s);
                break;
            
            //added this to surpress warning
            case PTR_TYPE:
                break;
            default:
                break;
    }

}
    
    
    
    
    
    
    
    
    
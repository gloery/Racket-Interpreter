//Written by Emily Johnston, Gordon Loery, and Charlotte Foran

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"
#include "tokenizer.h"
#include "parser.h"
#include "talloc.h"

Value *addToParseTree(Value*, int*, Value*);
void printValue(Value*);

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
    Value *tree = makeNull();
    int depth = 0;
    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    //loop through all tokens
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree,&depth,token);
        current = cdr(current);
    }
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses.\n");
        texit(EXIT_FAILURE);
    }
    //tree is backwards at this point, so reverse here
    tree = reverse(tree);
    return tree;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree){
    if (tree->type == NULL_TYPE) {
        return;
    }
    //if the current head of tree is a cons type, then go to its car
    else if (tree->type == CONS_TYPE) {
        //if the car is a cons type as well, enclose it in parentheses and recurse 
        if (car(tree)->type == CONS_TYPE) {
            printf("(");
            printTree(car(tree));
            printf(")");
            //adds a space after the end of a subtree (for formatting reasons)
            if (cdr(tree)->type != NULL_TYPE){
                printf(" ");
            }
            //recurse on the rest of the tree
            printTree(cdr(tree));
        }
        //if the car is not a cons type, print the car and recurse on the rest of the tree
        else{
            printTree(car(tree));
            if (cdr(tree)->type != NULL_TYPE){
                printf(" ");
            }
            printTree(cdr(tree));
        }
    } 
    //if the current head is not a cons type, print it
    else {
        printValue(tree);
        return;
    }
}

//print a non cons-type value based on its type
void printValue(Value *val) {
    switch(val->type){
            case NULL_TYPE:
                break;
            case CONS_TYPE:
                break;
            case INT_TYPE:
                printf("%i", val->i);
                break;
            case DOUBLE_TYPE:
                printf("%f", val->d);
                break;
            case STR_TYPE:
                printf("\"%s\"", val->s);
                break;
            case BOOL_TYPE:
                printf("%s", val->s);
                break;
            case OPEN_TYPE:
                break;
            case CLOSE_TYPE:
                break;
            case SYMBOL_TYPE:
                printf("%s", val->s);
                break;
            case PTR_TYPE:
                break;
            default:
                break;
    }
}


//Add given token to given tree, and update tree depth
Value *addToParseTree(Value *tree, int *depth, Value *token) {
    //if the token isn't a close type, cons it to the tree
    if (token->type != CLOSE_TYPE) {
        tree = cons(token, tree);
        //additionally, if it's an open type, increment depth
        if (token->type == OPEN_TYPE) {
            *depth = *depth + 1;
        }
    } 
    //if token is a close type, go backwards until we hit an open type
    //adding each token to a subtree as we go
    // and adding that subtree to the main tree at the end
    else {
        //check that there aren't too many close parens
        if (*depth == 0){
            printf("Syntax error: too many close parentheses. \n");
            texit(EXIT_FAILURE);
        }
        // if token type is close type
        //make a new list
        Value *subtree = makeNull();
        Value *cur = car(tree);
        tree = cdr(tree);
        //while the current token isn't a open paren, keep going, adding the
        //current token to the subtree and removing from original tree
        while (cur->type != OPEN_TYPE) {
            subtree = cons(cur, subtree);
            cur = car(tree);
            tree = cdr(tree);
        }
        //add the subtree to the original tree and decrement depth
        tree = cons(subtree, tree);
        *depth = *depth - 1;
    }
    return tree;
}

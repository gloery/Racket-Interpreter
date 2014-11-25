// linkedlist.c
// by Team Solid Spider: Emily Johnston, Gordon Loery, Charlotte Foran
// starter code provided by Dave Musicant
// part of the Racket Interpreter Project
// for CS 251: Programming Language Design and Implementation
#include "linkedlist.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "talloc.h"

void displayHelper(Value *);
void displayCons(Value *);

/*
 * Display the contents of the linked list to the screen
 * in some kind of readable format.
 */
void display(Value *list){
    // Print ' at beginning if top level is null, cons, or symbol
    if (list->type == CONS_TYPE ||
        list->type == NULL_TYPE ||
        list->type == SYMBOL_TYPE ) {
        printf("'");
        if(list->type == NULL_TYPE){
            printf("()");
        }
    }
    displayHelper(list);
}

void displayCons(Value *list) {
    Value *cur = list;
    while (cur->type == CONS_TYPE) {
        displayHelper(car(cur));
        cur = cdr(cur);
    }
}
    
void displayHelper(Value *list) { 
    Value *current = list;
        switch(current->type){
            case CONS_TYPE:
                // Display parentheses and call displayCons to handle Cons Cells
                printf("( ");
                displayCons(current);
                printf(") ");
                break;
            case INT_TYPE:
                printf("%i ", current->i);
                break;
            case DOUBLE_TYPE:
                printf("%f ", current->d);
                break;
            case STR_TYPE:
                printf("\"%s\" ", current->s);
                break;
            case PTR_TYPE:
                printf("%p ", current->p);
                break;
            case OPEN_TYPE:
                printf("( ");
                break;
            case CLOSE_TYPE:
                printf(") ");
                break;
            case SYMBOL_TYPE:
                printf("%s ", current->s);
                break;
            case BOOL_TYPE:
                printf("%s ", current->s);
                break;
            case CLOSURE_TYPE:
                printf("#<procedure> ");
            default:
                break;
        }
}

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *node;
    node = talloc(sizeof(Value));
    node->type = NULL_TYPE;
    return node;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *car, Value *cdr){
    if (car == NULL || cdr == NULL) {
        printf("Error: Cannot cons to null\n");
        texit(EXIT_FAILURE);
    }
    Value *node;
    node = talloc(sizeof(Value));
    node->type = CONS_TYPE;
    node->c.car = car;
    node->c.cdr = cdr;
    return node;
}

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list){
    Value *cur = list;
    // Create new list
    Value *new_list = makeNull();
    while (!isNull(cur)) {
        // Create new cons cell with pointers to old values/nested lists
        new_list = cons(car(cur), new_list);
        cur = cdr(cur);
    }
    return new_list;
}
// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
    assert (list->type == CONS_TYPE);
    return list->c.car;
}
// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
    assert (list->type == CONS_TYPE);
    return list->c.cdr;
}
// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
    assert (value != NULL);
    return value->type == NULL_TYPE;
}
// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    assert(value->type == CONS_TYPE || isNull(value));
    if (isNull(value)) {
        return 0;
    }
    return length(cdr(value)) + 1;
}
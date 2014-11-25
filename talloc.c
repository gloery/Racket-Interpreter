// talloc.c
// by Team Solid Spider: Emily Johnston, Gordon Loery, Charlotte Foran
// starter code provided by Dave Musicant
// part of the Racket Interpreter Project
// for CS 251: Programming Language Design and Implementation
#include "talloc.h"
#include <stdio.h>
// Simple linked list struct
struct Tlist {
    Value *v;
    struct Tlist *next;
};
typedef struct Tlist Tlist;
    
// Create head of active list
Tlist *head = NULL;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    // Create new Tlist node
    Tlist *new = malloc(sizeof(Tlist));
    // Allocate desired memory
    void *new_ptr = malloc(size);
    // Put pointer to allocated memory in active list
    new->v = new_ptr;
    new->next = head;
    head = new;
    // return the pointer
    return new_ptr;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
// Iterate through active list, free Tlist nodes and associated values
    Tlist *cur = head;
    while (cur != NULL){
        Tlist *temp = cur->next;
        free(cur->v);
        free(cur);
        cur = temp;
    }
    // Reset head
    head = cur;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}

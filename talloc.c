#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"

Value *head = NULL;
// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    if(head == NULL) {
        Value *next = malloc(sizeof(Value));
        next->p = malloc(size);
        head = next;
        (next->c).cdr = NULL;
        return next->p;
    }

    Value *current = head;
    while ((current->c).cdr != NULL) {
        current = (current->c).cdr;
    }
    
    Value *next = malloc(sizeof(Value));
    next->p = malloc(size);
    (current->c).cdr = next;
    (next->c).cdr = NULL;
    return next->p;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    Value *current = head;
    Value *next;
    while(current != NULL){
        next = (current->c).cdr;
        free(current->p);
        free(current);
        current = next;
    }
    head = NULL;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(status);
}

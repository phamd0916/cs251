#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Value myInt;
// myInt.type = INT_TYPE;
// myInt.i = 5;

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *node = talloc(sizeof(Value));
    node->type = NULL_TYPE;
    return node;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *node = talloc(sizeof(Value));
    node->type = CONS_TYPE;
    (node->c).car = newCar;
    (node->c).cdr = newCdr;
    return node;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list) {
    switch (list->type) {
        case INT_TYPE:
            printf("%i\n", list->i);
            break;
        case DOUBLE_TYPE:
            printf("%f\n", list->d);
            break;
        case STR_TYPE:
            printf("%s\n", list->s);
            break;
        case PTR_TYPE:
            break;
        case CONS_TYPE:
            printf("( ");
            display((list->c).car);
            printf(" ");
            display((list->c).cdr);
            printf(")");
            break;
        case NULL_TYPE:
            break;
        case OPEN_TYPE:
            printf("(");
            break;
        case CLOSE_TYPE:
            printf(")");
            break;
        case BOOL_TYPE:
            break;
        case SYMBOL_TYPE:
            break;
        default:
            break;
    }
}

// struct Value *copy(Value *list) {
//     if (list->type == NULL_TYPE){
//         return makeNull();
//     }
//     struct Value *new = talloc(sizeof(Value));
//     new->type = list->type;
//     switch (list->type) {
//         case INT_TYPE:
//             new->i = list->i;
//             break;
//         case DOUBLE_TYPE:
//             new->d = list->d;
//             break;
//         case STR_TYPE:
//             new->s = talloc((strlen(list->s) + 1) * sizeof(char));
//             strcpy(new->s, list->s);
//             break;
//         case PTR_TYPE:
//             new->p = list->p;
//         case CONS_TYPE:
//             (new->c).car = copy(car(list));
//         case NULL_TYPE:
//             break;
//         case OPEN_TYPE:
//             break;
//         case CLOSE_TYPE:
//             break;
//         case BOOL_TYPE:
//             break;
//         case SYMBOL_TYPE:
//             break;
//     }
//     return new;
// }
// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list){
    int length = 0;
    struct Value *new_head = makeNull();
    if(list->type == NULL_TYPE){
        return new_head;
    }
    struct Value *curr_length = list;
   
    while(curr_length->type != NULL_TYPE){
        curr_length = (curr_length->c).cdr;
        length++;
    }
    if(length == 0){
        return list;
    } 
    
    for(int i = 0; i < length; i++){
        struct Value *next = new_head;
        struct Value *curr = list;

        for(int j=0; j<i; j++){
            curr = cdr(curr);                
        }

        //printf("Type: %u\n", curr->type);
        new_head = talloc(sizeof(Value));
        new_head->type = CONS_TYPE;
        (new_head->c).car = car(curr);
        //display(new_head);
        (new_head->c).cdr = next;

    }
    
    return new_head;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
    return (list->c).car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    return (list->c).cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    if (value == NULL || value->type == NULL_TYPE ) {
        return true;
    }
    return false;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
    int count = 0;

    if (value->type == NULL_TYPE) {
        return 0;
    }

    if ((value->c).car->type != NULL_TYPE || (value->c).cdr->type != NULL_TYPE) {
        count ++;
    }
    if ((value->c).cdr->type == CONS_TYPE) {
        count += length((value->c).cdr);
    }
    
    return count;
}

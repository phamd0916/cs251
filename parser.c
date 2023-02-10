#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Adds a token to a parse tree
Value *addToParseTree(Value *tree, int *depth, Value *token) {
    // If token is an open paren, increments depth
    if (token->type == OPEN_TYPE) {
        tree = cons(token, tree);
        (*depth)++;
        return tree;
    }
    else {
        tree = cons(token, tree);
        return tree;
    }
}

// Prints error messages and exits for the two syntax error cases
void syntaxError(int case_num) {
    if (case_num == 1) {
        printf("Syntax error: too many close parentheses.\n");
        texit(1);
    }
    else if (case_num == 2) {
        printf("Syntax error: not enough close parentheses.\n");
        texit(1);
    }
}
// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    // creates empty linked list to contain parse trees
    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
    while (current->type != NULL_TYPE) {
        if ((*car(current)).type != CLOSE_TYPE) {
            Value *token = car(current);
            tree = addToParseTree(tree, &depth, token);
            current = cdr(current);
        }

        // when a list is made, added to the tree, and iterated through
        // the tree
        else {
            Value *currentNode = tree;
            Value *addOnList = makeNull();

            // adds tokens to a list being added to the tree until an open
            // parantheses is reached
            while (currentNode->type != NULL_TYPE && (*car(currentNode)).type != OPEN_TYPE) {
                Value *token = car(currentNode);
                addOnList = cons(token, addOnList);
                currentNode = cdr(currentNode);
            }

            // syntax error if the it reaches the bottom of the tree without
            // hitting an open parentheses
            if (currentNode->type == NULL_TYPE) {
                syntaxError(1);
            }

            // tree is updated to get rid of tokens and adds the lost to
            // the tress; depth gets decremented
            tree = cdr(currentNode);
            tree = cons(addOnList, tree);
            depth --;
            current = cdr(current);
            
        }
    }

    // depth should be at 0 when returning; if not there aren't enough
    // close parentheses and syntax error occurs.
    if (depth != 0) {
        syntaxError(2);
    }

    return reverse(tree);
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Value *tree) {
    Value *current = tree;
    while (current->type != NULL_TYPE) {
        if (current->type != CONS_TYPE) {
            switch (current->type) {
                case BOOL_TYPE:
                    if (current->i == 0) {
                        printf(". #f");
                    }
                    else {
                        printf(". #t");
                    }
                    break;
                case INT_TYPE:
                    printf(". %i", current->i);
                    break;
                case DOUBLE_TYPE:
                    printf(". %f", current->d);
                    break;
                case STR_TYPE:
                    printf(". \"%s\"", current->s);
                    break;
                case SYMBOL_TYPE:
                    printf(". %s", current->s);
                    break;
                default:
                    break;
            }
            break;
        }
        
        // printing nested parse trees
        if ((*car(current)).type == CONS_TYPE) {
            printf("(");
            printTree(car(current));
            printf(") ");
        }
        // printing empty lists
        else if ((*car(current)).type == NULL_TYPE) {
            printf("()");
        }

        // printing without a space
        else if ((*cdr(current)).type == NULL_TYPE) {
            Value *carValue = car(current);
            int carType = carValue->type;
            
            switch (carType) {
                case BOOL_TYPE:
                    if (carValue->i == 0) {
                        printf("#f");
                    }
                    else {
                        printf("#t");
                    }
                    break;
                case INT_TYPE:
                    printf("%i", carValue->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f", carValue->d);
                    break;
                case STR_TYPE:
                    printf("\"%s\"", carValue->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s", carValue->s);
                    break;
            }
        }

        // printing with a space
        else {
            Value *carValue = car(current);
            int carType = carValue->type;
            
            switch (carType) {
                case BOOL_TYPE:
                    if (carValue->i == 0) {
                        printf("#f ");
                    }
                    else {
                        printf("#t ");
                    }
                    break;
                case INT_TYPE:
                    printf("%i ", carValue->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f ", carValue->d);
                    break;
                case STR_TYPE:
                    printf("\"%s\" ", carValue->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s ", carValue->s);
                    break;
            }
        }
        current = cdr(current);
    }
}

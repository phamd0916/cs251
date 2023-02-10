#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "interpreter.h"
#include "talloc.h"
#include <string.h>

// looks for symbol type
Value *lookUpSymbol(Value *symbol, Frame *frame)
{
    // looks for symbol in each frame above the input frame
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        while (bindings == NULL) {
            if (frame->parent == NULL) {
                printf("Evaluation error\n");
                texit(1);
            }
            frame = frame->parent;
            bindings = frame->bindings;
        }
        while (bindings->type != NULL_TYPE)
        {
            Value *symbollCons = car(bindings);
            Value *symboll = car(symbollCons);
            assert(symboll->type == SYMBOL_TYPE);
            if (strcmp(symboll->s, symbol->s) == 0)
            {
                return car(cdr(symbollCons));
            }
            else
            {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }

    // if symbol is not found
    printf("Evaluation error: symbol 'x' not found.\n");
    texit(1);

    return symbol;
}

// Function that returns a true boolean Value
Value *trueVal() {
    Value *true_val = talloc(sizeof(Value));
    true_val->type = BOOL_TYPE;
    true_val->i = 1;
    return true_val;
}

// Function that returns a false boolean Value
Value *falseVal() {
    Value *false_val = talloc(sizeof(Value));
    false_val->type = BOOL_TYPE;
    false_val->i = 0;
    return false_val;
}

// evaluates every argument
Value *evalEach(Value *args, Frame *frame) {
   Value *cur = args;
   Value *evaled_args = makeNull();

   while (cur->type != NULL_TYPE && car(cur)->type != NULL_TYPE) {
       Value *arg = car(cur);
       Value *evaled_arg = eval(arg, frame);

       evaled_args = cons(evaled_arg, evaled_args);
       cur = cdr(cur);
   }
   
   // returns reversed list in order
   return reverse(evaled_args);
}

// evaluates if statement arguments
Value *evalIf(Value *args, Frame *frame)
{
    // checks if input is valid
    if (args->type != CONS_TYPE) {
        printf("Evaluation error: Not the correct number of arguments.\n");
        texit(1);
    }
    else {
        if (cdr(args)->type != CONS_TYPE) {
            printf("Evaluation error: Not the correct number of arguments.\n");
            texit(1);
        }
        else if (cdr(cdr(args))->type != CONS_TYPE) {
            printf("Evaluation error: Not the correct number of arguments.\n");
            texit(1);
        }
    }

    // creating values for boolean expressions and possible outcomes
    Value *boolExp = eval(car(args), frame);
    Value *truee = car(cdr(args));
    Value *falsee = car(cdr(cdr(args)));

    // checks input structure and returns proper outcomes
    if (boolExp->type == BOOL_TYPE) {
        if (boolExp->i == 1) {
            return eval(truee, frame);
        }
        else {
            return eval(falsee, frame);
        }
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    return args;
}

// evaluates let statement arguments
Value *evalLet(Value *args, Frame *frame)
{
    // creates new frame and sets input frame to parent frame
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = frame;
    newFrame->bindings = makeNull();

    // creates new linked list to store bindings
    Value *newBindings = newFrame->bindings;
    
    Value *current = car(args);

    if (car(current)->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }

    while (current->type != NULL_TYPE) {
        Value *evalNode = car(cdr(car(current)));
        Value *pointer = eval(evalNode, frame);
        Value *value = cons(pointer, makeNull());
        Value *symbolValue = car(car(current));

        if (symbolValue->type == SYMBOL_TYPE) {
            value = cons(symbolValue, value);
        }
        else {
            printf("Evaluation error\n");
            texit(1);
        }
        
        newBindings = cons(value, newBindings);
        current = cdr(current);
    }
    newFrame->bindings = newBindings;

    return eval(car(cdr(args)), newFrame);
}

// evaluates quote statement aruguments
Value *evalQuote(Value *args)
{
    // if there is no arguments
    if (isNull(args))
    {
        printf("Evaluation error\n");
        texit(1);
    }
    if (!isNull(cdr(args)))
    {
        printf("Evaluation error\n");
        texit(1);
    }

    return car(args);
}

// evaluates define statement arguements
Value *evalDefine(Value *args, Frame *frame)
{
    Value *var = car(args);
    Value *expr = car(cdr(args));

    // checks if there are too many arguments for define
    if ((cdr(cdr(args)))->type != NULL_TYPE)
    {
        printf("Evaluation error\n");
        texit(1);
    }

    // evaluating the expression and setting up in global frame
    Value *eval_expr = eval(expr, frame);
    Value *new_bindings = makeNull();
    new_bindings = cons(eval_expr, new_bindings);
    new_bindings = cons(var, new_bindings);
    frame->bindings = cons(new_bindings, frame->bindings);

    // returns void Value so interpreter ignores
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    return void_val;
}

// applies given function to multiple arguments
Value *apply(Value *function, Value *args) {
    assert(function->type == CLOSURE_TYPE || function->type == PRIMITIVE_TYPE);

    if (function->type == PRIMITIVE_TYPE) {
        return function->pf(args);
    }
    // creates closure
    struct Closure new = function->cl;

    // creates frame
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = new.frame;

    Value *new_bindings = makeNull();
    Value *cur_node = args;
    Value *params = new.paramNames;
    Value *cur_param = params;

    // setting up list of bindings based on parameters
    while (cur_node->type != NULL_TYPE) {
        if (cur_param->type == NULL_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }

        Value *list = makeNull();
        list = cons(car(cur_node), list);
        list = cons(car(cur_param), list);

        new_bindings = cons(list, new_bindings);

        cur_node = cdr(cur_node);
        cur_param = cdr(cur_param);
    }

    // if there isn't enough parameters
    if (cur_param->type != NULL_TYPE) {
         printf("Evaluation error hi\n");
         texit(1);
    }

    frame->bindings = new_bindings;
    Value *body = new.functionCode;

    return eval(body, frame);
}

// evaluates lambda statement arguments
Value *evalLambda(Value *args, Frame *frame) {
    // checks if input is valid
    if (args->type != CONS_TYPE)
    {
        printf("Evaluation error\n");
        texit(1);
    }

    Value *params = car(args);
    Value *body = car(cdr(args));
    
    struct Closure new;
    new.paramNames = params;
    new.functionCode = body;
    new.frame = frame;

    Value *closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl = new;

    return closure;
}

// evaluates let* statement arguements
Value *evalLetStar(Value *args, Frame *frame) {
    Frame *cur_frame = frame;
    
    Value *cur_node = car(args);
    // Check valid input structure
    if (car(cur_node)->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // Iterate through arguments...
    while (cur_node->type != NULL_TYPE) {
        Value *new_binding = makeNull();
        // Sets up singular var and value pair
        Value *node_to_eval = car(cdr(car(cur_node)));
        Value *pointer = eval(node_to_eval, cur_frame);
 
        Value *val = cons(pointer, makeNull());
        Value *symbol_val = car(car(cur_node));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            printf("Evaluation error\n");
            texit(1);
        }
        new_binding = cons(val, new_binding);
        
        Frame *new_frame = talloc(sizeof(Frame));
        new_frame->parent = cur_frame;
        new_frame->bindings = new_binding;
        
        // Iterative step to set up next var and value pair
        cur_frame = new_frame;
        cur_node = cdr(cur_node);
    }
    
    // Eval in new frame with new bindings
    return eval(car(cdr(args)), cur_frame);
}

// evaluates letrec statement arguements
Value *evalLetRec(Value *args, Frame *frame) {
    // Create new frame and set input frame to be parent frame
    Frame *new_frame = talloc(sizeof(Frame));
    new_frame->parent = frame;
    // Create new linked list to store bindings created in let statement
    new_frame->bindings = makeNull();
    Value *new_bindings = new_frame->bindings;
    
    Value *cur_node = car(args);
    // Check valid input structure
    if (car(cur_node)->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // Iterate through arguments...
    while (cur_node->type != NULL_TYPE) {
        // Sets up singular var and value pair
        Value *node_to_eval = car(cdr(car(cur_node)));
        Value *pointer = eval(node_to_eval, new_frame);
 
        Value *val = cons(pointer,makeNull());
        Value *symbol_val = car(car(cur_node));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            printf("Evaluation error\n");
            texit(1);
        }
        new_bindings = cons(val, new_bindings);
        
        // Iterative step to set up next var and value pair
        cur_node = cdr(cur_node);
    }
    
    new_frame->bindings = new_bindings;
    
    // Eval in new frame with new bindings
    return eval(car(cdr(args)), new_frame);
}

// evaluates set! statement arguements
Value *evalSet(Value *args, Frame *frame) {
    Frame *cur_frame = frame;
    Value *symbol = car(args);
    Value *new_val = eval(car(cdr(args)), frame);
    
    // Searches for symbol in each frame above input frame
    while (cur_frame != NULL) {
        Value *bindings = cur_frame->bindings;
        // While loop in case bindings member of frame not assigned
        while (bindings == NULL) {
            if (cur_frame->parent == NULL) {
                printf("Evaluation error\n");
                texit(1);
            }
            cur_frame = cur_frame->parent;
            bindings = cur_frame->bindings;
        }
        // Iterates through bindings...
        while (bindings->type != NULL_TYPE) {
            Value *symbol1_cons = car(bindings);
            Value *symbol1 = car(symbol1_cons);
            assert(symbol1->type == SYMBOL_TYPE);
            // Checks if string member of binding matches that of input symbol
            if (strcmp((*symbol1).s, (*symbol).s) == 0) {
                struct ConsCell cons_cell = symbol1_cons->c;
                cons_cell.cdr = cons(new_val, makeNull());
                symbol1_cons->c = cons_cell;
                
                Value *void_val = talloc(sizeof(Value));
                void_val->type = VOID_TYPE;
                return void_val;
            }
            // Otherwise continues search
            else {
                bindings = cdr(bindings);
            }
        }
        cur_frame = cur_frame->parent;
    }
    // If symbol not found, print evaluation error and exit
    printf("Evaluation error\n");
    texit(1);
    return symbol;
}

// evaluates begin statement arguements
Value *evalBegin(Value *args, Frame *frame) {
    // Evaluates every argument
    Value *evaledArgs = evalEach(args, frame);
    
    Value *cur_val = evaledArgs;
    // Goes through linked list to retrieve the final value and return it
    while (cur_val->type != NULL_TYPE) {
        if (cdr(cur_val)->type == NULL_TYPE) {
            return car(cur_val);
        }
        else {
            cur_val = cdr(cur_val);
        }
    }
    // Void Value used as stand in if the Begin isn't passed any arguments
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    return void_val;

}

// evaluates and statement arguements
Value *evalAnd(Value *args, Frame *frame) {
    // Searches through arguments, and returns false if any are false
    while (args->type != NULL_TYPE) {
        Value *bool_val = eval(car(args), frame);
        if (bool_val->type != BOOL_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }
        else if (bool_val->i == 0) {
            return falseVal();
        }
        else {
            args = cdr(args);
        }
    }
    // Returns true if all values are true
    return trueVal();
}

// evaluates or statement arguements
Value *evalOr(Value *args, Frame *frame) {
    // If there is a true in argument list, return true
    while (args->type != NULL_TYPE) {
        Value *bool_val = eval(car(args), frame);
        if (bool_val->type != BOOL_TYPE) {
            // Error if the argument doesn't resolve to a boolean
            printf("Evaluation error\n");
            texit(1); 
        }
        else if (bool_val->i == 1) {
            return trueVal();
        }
        else {
            args = cdr(args);
        }
    }
    // Return false if the arguments are all false
    return falseVal();
}

// evaluates cond statement arguements
Value *evalCond(Value *args, Frame *frame) {
    while (args->type != NULL_TYPE) {
        Value *bool_val;
        // If the argument is a symbol, test if it is else
        if (car(car(args))->type == SYMBOL_TYPE) {
            if (strcmp(car(car(args))->s, "else") == 0) {
                // else sets the boolean to true
                bool_val = trueVal();
            }
            else {
                printf("Evaluation error\n");
                texit(1);
            }
        }
        else {
            // Need to evaluate otherwise
            bool_val = eval(car(car(args)), frame);
        }
        if (bool_val->type != BOOL_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }
        else if (bool_val->i == 1) {
            // Return the accompanying code if the bool is true
            return eval(car(cdr(car(args))), frame);
        }
        else {
            // Otherwise, move down list
            args = cdr(args);
        }
    }
    // If nothing is true, we return a void Value
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    return void_val;
}

// Sets initial result value and add each succesive
// value in args list
Value *primitiveAdd(Value *args) {
    float result = 0;
    Value *result_val = talloc(sizeof(Value));

    if (isNull(args)) {
        result_val->type = INT_TYPE;
        result_val->i = result;
        return result_val;
    }
    
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                // Throws error if the argument isn't a number
                printf("Evaluation error\n");
                texit(1);
            }
            result = result + cur_node->d;
            args = cdr(args);
            result_val->type = DOUBLE_TYPE;
            result_val->d = result;
        }
        else {
            result = result + cur_node->i;
            args = cdr(args);
            result_val->type = INT_TYPE;
            result_val->i = result;
        }
    }
    
    return result_val;
}

// checks if args is null and returns true if null
// and false if not null
Value *primitiveNull(Value *args) {
    // The argument passed should not be empty
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    assert(args->type == CONS_TYPE);
    // If the cdr isn't empty, then they have passed more than one argument
    if (cdr(args)->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        // Calls method that returns true bool val
        return trueVal();
    }
    else {
        // Equivalent method for false bool val
        return falseVal();
    }
}

// returns the first item or car of a list
Value *primitiveCar(Value *args) {
    // Cannot call car of a nonexistent argument
    if (args->type == NULL_TYPE){
        printf("Evaluation error\n");
        texit(1);
    }
    // Cannot have more than one argument
    if (isNull(car(args)) || car(args)->type != CONS_TYPE || !isNull(cdr(args))) {
        printf("Evaluation error\n");
        texit(1);
    }    

    // takes the first argument, and returns its car
    Value *argument = car(args);
    return car(argument);
}

// Returns all items except first in list
Value *primitiveCdr(Value *args) {
    // Cannot call car of a nonexistent argument
    if (args->type == NULL_TYPE){
        printf("Evaluation error\n");
        texit(1);
    }
    assert(args->type == CONS_TYPE);
    // Cannot have more than one argument
    if (cdr(args)->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    Value *lst = car(args);
    if (lst->type != CONS_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // Returns the cdr of the first argument
    return cdr(lst);
}

// Creates pair between next two arguments
Value *primitiveCons(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    //The first argument to put in cons cell will always be car(args)
    Value *arg1 = car(args);
    Value *arg2;
    
    if (cdr(args)->type == CONS_TYPE) {
        arg2 = car(cdr(args));
    }
    // If the cdr of args isn't a Cons type, we simply set arg2 as the cdr
    else {
        arg2 = cdr(args);
    }
    Value *result_val = cons(arg1, arg2);
    return result_val;
}

// built in subtraction
Value *primitiveSubtract(Value *args) {
    // Create result to return
    float result = 0;
    Value *result_val = talloc(sizeof(Value));
    
    // Add first argument to result
    Value *cur_node = car(args);
    if (cur_node->type != INT_TYPE) {
        if (cur_node->type != DOUBLE_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }
        result = result + cur_node->d;
        args = cdr(args);
    }
    else {
        result = result + cur_node->i;
        args = cdr(args);
    }
    
    // And subtract subsequent arguments
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                printf("Evaluation error\n");
                texit(1);
            }
            result = result - cur_node->d;
            args = cdr(args);
            result_val->type = DOUBLE_TYPE;
            result_val->d = result;

        }
        else {
            result = result - cur_node->i;
            args = cdr(args);
            result_val->type = INT_TYPE;
            result_val->i = result;
        }
    }
    
    
    return result_val;
}

// built in functions that checks if left argument is the same as the right
Value *primitiveEquals(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Create float versions of two args for comparison
    float arg1;
    float arg2;
    
    // Assign an int or double to float for comparison
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Perform comparison and return corresponding boolean
    if (arg1 == arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

// built in functions that checks if left argument is greater than right
Value *primitiveGreaterThan(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Create float versions of two args for comparison
    float arg1;
    float arg2;
    
    // Assign an int or double to float for comparison
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Perform comparison and return corresponding boolean
    if (arg1 > arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

// built in functions that checks if left argument is less than right
Value *primitiveLessThan(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Create float versions of two args for comparison
    float arg1;
    float arg2;
    
    // Assign an int or double to float for comparison
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (float) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (float) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        printf("Evaluation error\n");
        texit(1);
    }
    
    // Perform comparison and return corresponding boolean
    if (arg1 < arg2) {
        return trueVal();
    }
    else {
        return falseVal();
    }
}

// built in multiplication
Value *primitiveMultiply(Value *args){
    
    float result = 1;
    Value *result_val = talloc(sizeof(Value));
    
    // Set first argument to result
    Value *cur_node = car(args);
    if (cur_node->type != INT_TYPE) {
        if (cur_node->type != DOUBLE_TYPE) {
            printf("Evaluation error\n");
            texit(1);
        }
        result = result * cur_node->d;
        args = cdr(args);
    }
    else {
        result = result * cur_node->i;
        args = cdr(args);
    }
    
    // Continue to multiply subsequent arguments
    while (args->type != NULL_TYPE) {
        Value *cur_node = car(args);
        if (cur_node->type != INT_TYPE) {
            if (cur_node->type != DOUBLE_TYPE) {
                printf("Evaluation error\n");
                texit(1);
            }
            result = result * cur_node->d;
            args = cdr(args);
            result_val->type = DOUBLE_TYPE;
            result_val->d = result;

        }
        else {
            result = result * cur_node->i;
            args = cdr(args);
            result_val->type = INT_TYPE;
            result_val->i = result;
        }
    }
    return result_val;
}

// built in divison, will return a float if numbers to not devide evenly
Value *primitiveDivide(Value *args){
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }

    Value *result_val = talloc(sizeof(Value));
    Value *cur_node = car(args);
    Value *next_node = car(cdr(args));

    if (cur_node->type == INT_TYPE) {
        if (next_node->type == INT_TYPE) {
            if (cur_node->i % next_node->i == 0) {
                result_val->type = INT_TYPE;
                result_val->i = cur_node->i/next_node->i;
            }
            else {
                result_val->type = DOUBLE_TYPE;
                result_val->d = cur_node->i/(float)next_node->i;
            }
        }
        else {
            result_val->type = DOUBLE_TYPE;
            result_val->d = cur_node->i/(float)next_node->i;
        }
    }

    else {
        result_val->type = DOUBLE_TYPE;
        if (next_node->type == DOUBLE_TYPE) {
            result_val->d = cur_node->d/next_node->d;
        }
        else {
            result_val->d = cur_node->d/next_node->i;
        }
        
    }
    
    return result_val;
}

// built in modulo function, takes in an integer.
Value *primitiveModulo(Value *args){
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (car(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }
    // If there exists a third argument, throw an Error
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        printf("Evaluation error\n");
        texit(1);
    }

    Value *result_val = talloc(sizeof(Value));
    result_val->type = INT_TYPE;
    result_val->i = (car(args)->i) % (car(cdr(args))->i);
    return result_val;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;

    Value *symbol = talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;

    Value *binding = makeNull();
    binding = cons(value, binding);
    binding = cons(symbol, binding);

    frame->bindings = cons(binding, frame->bindings);
}

// interprets the tree
void interpret(Value *tree)
{
    Frame *global = talloc(sizeof(Frame));
    global->bindings = makeNull();
    global->parent = NULL;

    bind("+", primitiveAdd, global);
    bind("null?", primitiveNull, global);
    bind("car", primitiveCar, global);
    bind("cdr", primitiveCdr, global);
    bind("cons", primitiveCons, global);
    bind("-", primitiveSubtract, global);
    bind("=", primitiveEquals, global);
    bind(">", primitiveGreaterThan, global);
    bind("<", primitiveLessThan, global);
    bind("*", primitiveMultiply, global);
    bind("/", primitiveDivide, global);
    bind("modulo", primitiveModulo, global);

    while (!isNull(tree))
    {   
        Value *expression = car(tree);
        Value *result = eval(expression, global);

        // prints resulting Value appropriately
        switch (result->type)
        {
        case BOOL_TYPE:
            if (result->i == 0)
            {
                printf("#f\n");
            }
            else
            {
                printf("#t\n");
            }
            break;
        case INT_TYPE:
            printf("%i\n", result->i);
            break;
        case DOUBLE_TYPE:
            printf("%f\n", result->d);
            break;
        case STR_TYPE:
            printf("\"%s\"\n", result->s);
            break;
        case SYMBOL_TYPE:
            printf("%s\n", result->s);
            break;
        case CONS_TYPE:
            printf("(");
            printTree(result);
            printf(")\n");
            break;
        case NULL_TYPE:
            printf("()\n");
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>\n");
            break;
        case PRIMITIVE_TYPE:
            printf("#<procedure>\n");
            break;
        default:
            break;
        }
        tree = cdr(tree);
    }
}

// evaluates Scheme program
Value *eval(Value *tree, Frame *frame)
{
    Value *result;
    switch (tree->type)
    {
    case INT_TYPE:
    {
        result = tree;
        break;
    }
    case DOUBLE_TYPE:
    {
        result = tree;
        break;
    }
    case STR_TYPE:
    {
        result = tree;
        break;
    }
    case BOOL_TYPE:
    {
        result = tree;
        break;
    }
    case SYMBOL_TYPE:
    {
        return lookUpSymbol(tree, frame);
        break;
    }
    case CONS_TYPE:
    {
        Value *first = car(tree);
        Value *args = cdr(tree);

        // Sanity and error checking on first...
        if (first->type == SYMBOL_TYPE) {
            if (!strcmp(first->s, "if")) {
                result = evalIf(args, frame);
            }
            else if (!strcmp(first->s, "let")) {
                result = evalLet(args, frame);
            }
            else if (!strcmp(first->s, "quote")) {
                result = evalQuote(args);
            }
            else if (!strcmp(first->s, "define")) {
                result = evalDefine(args, frame);
            }
            else if (!strcmp(first->s, "lambda")) {
                result = evalLambda(args, frame);
            }
            else if (strcmp(first->s, "let*") == 0) {
                result = evalLetStar(args, frame);
            }
            else if (strcmp(first->s, "letrec") == 0) {
                result = evalLetRec(args, frame);
            }
            else if (strcmp(first->s, "set!") == 0) {
                result = evalSet(args, frame);
            }
            else if (strcmp(first->s, "begin") == 0) {
                result = evalBegin(args, frame);
            }
            else if (strcmp(first->s, "and") == 0) {
                result = evalAnd(args, frame);
            }
            else if (strcmp(first->s, "or") == 0) {
                result = evalOr(args, frame);
            }
            else if (strcmp(first->s, "cond") == 0) {
                    result = evalCond(args, frame);
            }

            // .. other special forms here...

            else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator, evaledArgs);
            }
        }

        else {
                // If not a special form, evaluate the first, evaluate the args, then
                // apply the first to the args.
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = evalEach(args, frame);
                return apply(evaledOperator, evaledArgs);
        }

        break;
    }

    default: {
        printf("Evaluation error: Not supported Value type %u \n", tree->type);
        texit(1);
    }
    }
    return result;
}

// interpreter.c
// by Team Solid Spider: Emily Johnston, Gordon Loery, Charlotte Foran
// starter code provided by Dave Musicant
// part of the Racket Interpreter Project
// for CS 251: Programming Language Design and Implementation
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "interpreter.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"
#include "parser.h"

Frame *newFrame(Frame *);

/*** Main Functions ***/
Value *evalEach(Value*, Frame*);
//eval and interpret included in header file

/*** Primitives ***/
void bindPrimitives(Frame *);
void bind(char *name, Value *(*function)(struct Value *), Frame *);
Value *primitiveAdd(Value *);
Value *primitiveSub(Value *);
Value *primitiveMult(Value *);
Value *primitiveDiv(Value *);
Value *primitiveGre(Value *);
Value *primitiveLess(Value *);
Value *primitiveEq(Value *);
Value *primitiveMod(Value *);
Value *primitiveNull(Value *);
Value *nullHelper(Value *);
Value *primitiveCar(Value *);
Value *primitiveCdr(Value *);
Value *primitiveCons(Value *); 

/*** Special Forms ***/
Value *evalLet(Value*, Frame*);
Value *evalLetStar(Value*, Frame*);
Value *evalLetRec(Value*, Frame*);
Value *evalQuote(Value*);
Value *evalIf(Value*, Frame*);
Value *evalDefine(Value*, Frame*);
Value *evalLambda(Value*, Frame*);
Value *evalBegin(Value*, Frame*);
Value *evalSet(Value*, Frame*);
Value *evalAnd(Value*, Frame*);
Value *evalOr(Value*, Frame*);
Value *evalCond(Value*, Frame*);

/*** Functions and Symbols ***/
Value *apply(Value*, Value*);
Value *lookUpSymbol(Value*, Frame*);


Frame *newFrame(Frame* parent) {
    Frame *f = talloc(sizeof(Frame));
    f->parent = parent;
    f->bindings = makeNull();
    
    return f;
}


/**********************/
/*** Main Functions ***/
/**********************/


/*
 * Creates top level frame, evaluates each expression of 
 * the input, and prints the result
 */
void interpret(Value *list) {
    // Create global/top level frame
    Frame *topFrame = newFrame(NULL);
    
    bindPrimitives(topFrame);
    // Iterate through each expression in program and
    // display result of that evaluation.
    Value *cur = list;
    while(cur->type != NULL_TYPE){
        Value *result = eval(car(cur), topFrame);
        if (result->type != VOID_TYPE) {
            display(result);
            printf("\n");
        }
        cur = cdr(cur);
    }
}

/*
 * Evaluates current tree with frame as environment.
 */
Value *eval(Value *tree, Frame *frame) {
    switch (tree->type)  {
        // Integer, boolean, string, and double all evaluate to themselves
        case INT_TYPE:
            return tree;
            break;
        case BOOL_TYPE:
            return tree;
            break;
        case STR_TYPE:
            return tree;
            break;
        case DOUBLE_TYPE:
            return tree;
            break;
        // When we encounter a symbol, look it up 
        // and return the value associated with it
        case SYMBOL_TYPE: {
            Value *toReturn = talloc(sizeof(Value));
            toReturn = lookUpSymbol(tree, frame);
            return toReturn;
            break;
        }
        // Here to suppress warnings.
        case OPEN_TYPE:
            return tree;
            break;
        case CLOSE_TYPE:
            return tree;
            break;
        case PTR_TYPE:
            return tree;
            break;
        case NULL_TYPE:
            return tree;
            break;
        // If we get to a cons type, check first thing
        case CONS_TYPE:{
            Value *first = talloc(sizeof(Value));
            first = car(tree);
            Value *args = talloc(sizeof(Value));
            args = cdr(tree);
            Value *result = talloc(sizeof(Value));

            // Special Forms
            // If first thing in cons is a symbol or cons type, continue
            if (first->type == SYMBOL_TYPE || first->type == CONS_TYPE) {
                if (!strcmp(first->s,"if")) {
                    result = evalIf(args, frame);
                }
                else if(!strcmp(first->s,"let")){
                    result = evalLet(args, frame);
                }
                else if(!strcmp(first->s, "let*")){
                    result = evalLetStar(args, frame);
                }
                else if(!strcmp(first->s, "letrec")){
                    result = evalLetRec(args, frame);
                }
                else if (!strcmp(first->s, "quote")) {
                    result = evalQuote(tree);
                }
                else if (!strcmp(first->s, "define")) {
                    result = evalDefine(args, frame);
                }
                else if (!strcmp(first->s, "lambda")) {
                    result = evalLambda(args, frame);
                }
                else if(!strcmp(first->s, "begin")){
                    result = evalBegin(args, frame);
                }
                else if(!strcmp(first->s, "set!")){
                    result = evalSet(args, frame);
                }
                else if (!strcmp(first->s, "and")) {
                    result = evalAnd(args, frame);
                }
                else if (!strcmp(first->s, "or")) {
                    result = evalOr(args, frame);
                }
                else if (!strcmp(first->s, "cond")) {
                    result = evalCond(args, frame);
                }
                // Anything else
                else {

                    Value *evaledOperator = eval(first, frame);

                    // If first is a Racket function
                    if (evaledOperator->type == CLOSURE_TYPE) {
                        Value *evaledArgs = evalEach(args, frame);
                        result = apply(evaledOperator, evaledArgs);
                    } 
                    // If first is a primitive function
                    else if (evaledOperator->type == PRIMITIVE_TYPE) {
                        Value *evaledArgs = evalEach(args, frame);
                        // apply primitive function to previously evaled args
                        result = evaledOperator->pf(evaledArgs);
                    }
                    // If first is not recognized, and is a symbol type
                    else if (evaledOperator->type == SYMBOL_TYPE){
                        printf("Evaluation error: This is not a recognized procedure.\n");
                        texit(EXIT_FAILURE);
                    }
                    //The case where first contained a cons type that does not evaluate to a symbol, closure, or primitive type.
                    else{
                        return tree;
                    }
                }
            } 
            // If first thing is not a symbol or cons type, it's from inside a quote; return it
            else {
                return tree;
            }
            return result;
            break;
        }
        default:
            return tree;
            break;
    }
}

/*
 * Used to evaluate each argument passed to a function.
 * Returns a list of the evaluated arguments.
 */
Value *evalEach(Value *args, Frame *frame) {
    Value *cur = args;
    Value *evaled = makeNull();
    // Evaluate each argument, then add it to evaled list
    while (cur->type != NULL_TYPE) {
        evaled = cons(eval(car(cur), frame), evaled);
        cur = cdr(cur);
    }
    // The evaled list will be in reverse order, so reverse it back and return.
    evaled = reverse(evaled);
    return evaled;
}



/******************/
/*** Primitives ***/
/******************/


/*
 * Helper function to bind all supported primitive
 * function names to their functions.
 */
void bindPrimitives(Frame *frame){
    bind("+", primitiveAdd, frame);
    bind("-", primitiveSub, frame);
    bind("*", primitiveMult, frame);
    bind("/", primitiveDiv, frame);
    bind(">", primitiveGre, frame);
    bind("<", primitiveLess, frame);
    bind("=", primitiveEq, frame);
    bind("modulo", primitiveMod, frame);
    bind("null?", primitiveNull, frame);
    bind("car", primitiveCar, frame);
    bind("cdr", primitiveCdr, frame);
    bind("cons", primitiveCons, frame);
}

/*
 * Binds a primitive function name to its function pointer.
 */
void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    
    Value *nameHolder = talloc(sizeof(Value));
    nameHolder->type = SYMBOL_TYPE;
    nameHolder->s = name;
    
    // Add primitive functions to top-level bindings list
    Value *value = talloc(sizeof(Value));
    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    Value *binding = makeNull();
    binding = cons(value, binding);
    binding = cons(nameHolder, binding);
    frame->bindings = cons(binding, frame->bindings);
}

/*
 * Primitive function to add two values in Racket.
 */
Value *primitiveAdd(Value *addList){
    double runningTotal = 0.0;
    //check number of args, if 0 return 0, if 1 return that, otherwise add them
    
    //loop through all arguments, add them
    while(addList->type != NULL_TYPE){
        Value *number = car(addList);
        //check to make sure args are ints or doubles
        if (number->type != INT_TYPE &&
           number->type != DOUBLE_TYPE) {
            printf("Error: I can't add this!\n");
            texit(EXIT_FAILURE);
        }
        if(number->type == INT_TYPE){
            runningTotal += number->i;
        }
        else {
            runningTotal += number->d;
        }
        addList = cdr(addList);
    }
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = runningTotal;
    
    return total;
}

/*
 * Primitive function to subtract values in Racket.
 */
Value *primitiveSub(Value *subList){
    //make sure there are two things to subtract
    if (subList->type != CONS_TYPE ||
        cdr(subList)->type != CONS_TYPE ||
        cdr(cdr(subList))->type != NULL_TYPE){
            printf("Wrong number of arguments for subtract\n");         texit(EXIT_FAILURE);
    }
    double toReturn = 0.0;
    Value *firstNum = car(subList);
    Value *secondNum = car(cdr(subList));
    // Check that inputs are ints or doubles
    if((firstNum->type != INT_TYPE && firstNum->type != DOUBLE_TYPE) || (secondNum->type != INT_TYPE && secondNum->type != DOUBLE_TYPE)){
            printf("Error: I can't subtract this!\n");
            texit(EXIT_FAILURE);
        }
    //isolate first thing
    if(firstNum->type == INT_TYPE){
            toReturn += firstNum->i;
        }
        else{
            toReturn += firstNum->d;
        }
    //subtract
    if(secondNum->type == INT_TYPE){
            toReturn -= secondNum->i;
        }
        else{
            toReturn -= secondNum->d;
        }
    Value *final = talloc(sizeof(Value));
    final->type = DOUBLE_TYPE;
    final->d = toReturn;
    
    return final;
}

/*
 * Primitive function to multiply values in Racket.
 */
Value *primitiveMult(Value *multList){
    double runningTotal = 1.0;
    //loop through all arguments, multiply
    while(multList->type != NULL_TYPE){
        Value *number = car(multList);
        
        //check to make sure args are ints or doubles
        if(number->type != INT_TYPE && number->type != DOUBLE_TYPE){
            printf("Error: I can't multiply this!\n");
            texit(EXIT_FAILURE);
        }
        //multiply number with running total
        if(number->type == INT_TYPE){
            runningTotal *= number->i;
        }
        else{
            runningTotal *= number->d;
        }
        multList = cdr(multList);
    }
    Value *total = talloc(sizeof(Value));
    total->type = DOUBLE_TYPE;
    total->d = runningTotal;
    
    return total;
}

/*
 * Primitive function to divide in Racket.
 */
Value *primitiveDiv(Value *nums){
    //check number of args is two
    if (nums->type != CONS_TYPE ||
        cdr(nums)->type != CONS_TYPE ||
        cdr(cdr(nums))->type != NULL_TYPE){
            printf("Wrong number of arguments for comparison\n");
        texit(EXIT_FAILURE);
    }
    
    double getFirst = 0.0;
    double getSecond = 0.0;
    Value *firstNum = car(nums);
    Value *secondNum = car(cdr(nums));
    
    //check both args are ints or doubles
    if((firstNum->type != INT_TYPE && firstNum->type != DOUBLE_TYPE) || (secondNum->type != INT_TYPE && secondNum->type != DOUBLE_TYPE)){
            printf("Error: I can't divide these!\n");
            texit(EXIT_FAILURE);
        }
    //make both args into doubles
    if(firstNum->type == INT_TYPE){
            getFirst += firstNum->i;
        }
    else{
            getFirst += firstNum->d;
        }
    if(secondNum->type == INT_TYPE){
        getSecond += secondNum->i;
        }
    else{
        getSecond += secondNum->d;   
        }
    //divide numbers
    Value *final = talloc(sizeof(Value));
    final->type = DOUBLE_TYPE;
    final->d = getFirst/getSecond;
    return final;
}

/*
 * Primitive function to check greater than in Racket.
 */
Value *primitiveGre(Value *nums){
    //check number of args is 2
    if (nums->type != CONS_TYPE ||
        cdr(nums)->type != CONS_TYPE ||
        cdr(cdr(nums))->type != NULL_TYPE){
            printf("Wrong number of arguments for comparison\n");  
        texit(EXIT_FAILURE);
    }
    
    Value *final = talloc(sizeof(Value));
    final->type = BOOL_TYPE;
    double getFirst = 0.0;
    Value *firstNum = car(nums);
    Value *secondNum = car(cdr(nums));
    
    //check both args are ints or doubles
    if((firstNum->type != INT_TYPE && firstNum->type != DOUBLE_TYPE) || (secondNum->type != INT_TYPE && secondNum->type != DOUBLE_TYPE)){
            printf("Error: I can't compare these!\n");
            texit(EXIT_FAILURE);
        }
    //isolate first arg
    if(firstNum->type == INT_TYPE){
            getFirst += firstNum->i;
        }
    else{
            getFirst += firstNum->d;
        }
    //compare args, change final value to return
    if(secondNum->type == INT_TYPE){
            if (getFirst > secondNum->i){
                final->s = "#t";
            }
            else{
                final->s = "#f"; 
            }
        }
    else{
            if (getFirst > secondNum->d){
                final->s = "#t";
            }
            else{
                final->s = "#f"; 
            }
        }
    
    return final;
}

/*
 * Primitive function to check less than in Racket.
 */
Value *primitiveLess(Value *nums){
    //check number of args is two
    if (nums->type != CONS_TYPE ||
        cdr(nums)->type != CONS_TYPE ||
        cdr(cdr(nums))->type != NULL_TYPE){
            printf("Wrong number of arguments for comparison\n");  
        texit(EXIT_FAILURE);
    }
    Value *final = talloc(sizeof(Value));
    final->type = BOOL_TYPE;
    double getFirst = 0.0;
    Value *firstNum = car(nums);
    Value *secondNum = car(cdr(nums));
    
    //check both args are ints or doubles
    if((firstNum->type != INT_TYPE && firstNum->type != DOUBLE_TYPE) || (secondNum->type != INT_TYPE && secondNum->type != DOUBLE_TYPE)){
            printf("Error: I can't compare these!\n");
            texit(EXIT_FAILURE);
        }
    
    //isolate first thing
    if(firstNum->type == INT_TYPE){
            getFirst += firstNum->i;
        }
    else{
            getFirst += firstNum->d;
        }
    
    //make comparison, change final value to return
    if(secondNum->type == INT_TYPE){
            if (getFirst < secondNum->i){
                final->s = "#t";
            }
            else{
                final->s = "#f"; 
            }
        }
    else{
            if (getFirst < secondNum->d){
                final->s = "#t";
            }
            else{
                final->s = "#f"; 
            }
        }
    return final;
}

/*
 * Primitive function to check if values are equal in Racket.
 */
Value *primitiveEq(Value *nums){
    
    //check number of args is two
    if (nums->type != CONS_TYPE ||
        cdr(nums)->type != CONS_TYPE ||
        cdr(cdr(nums))->type != NULL_TYPE){
            printf("Wrong number of arguments for comparison\n");
        texit(EXIT_FAILURE);
    }
    
    double getFirst = 0.0;
    double getSecond = 0.0;
    Value *firstNum = car(nums);
    Value *secondNum = car(cdr(nums));
    
    //check both args are ints or doubles
    if((firstNum->type != INT_TYPE && firstNum->type != DOUBLE_TYPE) || (secondNum->type != INT_TYPE && secondNum->type != DOUBLE_TYPE)){
            printf("Error: I can't compare these!\n");
            texit(EXIT_FAILURE);
        }
    
    //make both args into doubles
    if(firstNum->type == INT_TYPE){
            getFirst += firstNum->i;
        }
    else{
            getFirst += firstNum->d;
        }
    if(secondNum->type == INT_TYPE){
        getSecond += secondNum->i;
        }
    else{
        getSecond += secondNum->d;   
        }
    
    Value *final = talloc(sizeof(Value));
    final->type = BOOL_TYPE;
    //if one arg is greater than the other, they are not equal
    if (getFirst < getSecond || getFirst > getSecond){
        final->s = "#f";
    }
    else{
        final->s = "#t";
    }
    return final;
}

/*
 * Primitive function to take the mod of given args in Racket.
 */
Value *primitiveMod(Value *nums){
    //check number of args is two
    if (nums->type != CONS_TYPE ||
        cdr(nums)->type != CONS_TYPE ||
        cdr(cdr(nums))->type != NULL_TYPE){
            printf("Wrong number of arguments for mod\n");                 texit(EXIT_FAILURE);
    }
    
    Value *final = talloc(sizeof(Value));
    final->type = INT_TYPE;
    Value *firstNum = car(nums);
    Value *secondNum = car(cdr(nums));
    
    //check that both args are ints
    if(firstNum->type != INT_TYPE || secondNum->type != INT_TYPE){
            printf("Error: I can't mod these!\n");
            texit(EXIT_FAILURE);
        }
    //mod args
    int getFirst = firstNum->i;
    int getSecond = secondNum->i;
    final->i = getFirst % getSecond;
    
    return final;
}

/*
 * Primitive function to check if the given argument is null in Racket.
 */
Value *primitiveNull(Value *args) {
    if(args->type == NULL_TYPE){
        printf("Error: Wrong number of args for null check.\n");
        texit(EXIT_FAILURE);
    }
    //verify that there is only one arg
    if(args->type == CONS_TYPE){
        if(cdr(args)->type == CONS_TYPE && car(cdr(args))->type != NULL_TYPE){
            printf("Error: Wrong number of args for null check.\n");
            texit(EXIT_FAILURE);
        }
        else if(cdr(args)->type != NULL_TYPE){
            printf("Error: Wrong number of args for null check.\n");
            texit(EXIT_FAILURE);
        }
    }
    return nullHelper(args);
}

/*
 * Recursive helper function for primitiveNull that actually checks for null.
 */
Value *nullHelper(Value *args){
    //check to see if we're in a cons cell, if so, go deeper
    if (args->type == CONS_TYPE){
        return nullHelper(car(args));
    }
    //base case: return if the innermost thing is null or not
    else{
        Value *result = makeNull();
        result->type = BOOL_TYPE;
        if (isNull(args)) {
            result->s = "#t";
        } else {
            result->s = "#f";
        }
        return result;
    }
}

/*
 * Primitive function to return the car (head) of a linked list in Racket.
 */
Value *primitiveCar(Value *args){
    //verify correct number and type of args
    if(args->type == NULL_TYPE){
        printf("Error: Wrong number of args for getting car.\n");
        texit(EXIT_FAILURE);
    }
    if (args->type != CONS_TYPE) {
        printf("Error: Can't call car on this type.\n");
        texit(EXIT_FAILURE);
    }
    if (cdr(args)->type != NULL_TYPE) {
        printf("Error: Wrong number of args for getting car.\n");
        texit(EXIT_FAILURE);
    }
    if (car(args)->type != CONS_TYPE) {
        printf("Error: Can't get car.\n");
        texit(EXIT_FAILURE);
    }

    //return the car of the thing
    Value *result = car(car(args));
    return result;
}

/*
 * Primitive function to return the cdr (tail) of a linked list in Racket.
 */
Value *primitiveCdr(Value *args){
    //verify correct number and type of args
    if(args->type == NULL_TYPE){
        printf("Error: Wrong number of args for getting cdr.\n");
        texit(EXIT_FAILURE);
    }
    if (args->type != CONS_TYPE) {
        printf("Error: Can't call cdr on this type.\n");
        texit(EXIT_FAILURE);
    }
    if (cdr(args)->type != NULL_TYPE) {
        printf("Error: Wrong number of args for getting cdr.\n");
        texit(EXIT_FAILURE);
    }
    if (car(args)->type != CONS_TYPE) {
        printf("Error: Can't get cdr.\n");
        texit(EXIT_FAILURE);
    }

    //return the car of the thing
    Value *result = cdr(car(args));
    return result;
}

/*
 * Primitive function to implement cons in Racket.
 */
Value *primitiveCons(Value *args){

    //verify type and number of args
    if(args->type == NULL_TYPE){
        printf("Error: Wrong number of args for creating cons cell.\n");
        texit(EXIT_FAILURE);
    }
     
    if (args->type != CONS_TYPE) {
        printf("Error: Can't call cdr on this type.\n");
        texit(EXIT_FAILURE);
    }

    if (cdr(args)->type != CONS_TYPE ||
        cdr(cdr(args))->type != NULL_TYPE){
        printf("Error: Can't cons this.\n");
        texit(EXIT_FAILURE);
    }
    Value *result;
    //return the result of consing the first arg onto the second one
    if (car(cdr(args))->type == CONS_TYPE){
        result = cons(car(args), car(cdr(args)));
    }
    else {
        result = cons(car(args), cdr(args));
    }
    return result;
    
}



/*********************/
/*** Special Forms ***/
/*********************/


/*
 * Evaluates a let expression with arguments args in environment frame.
 */
Value *evalLet(Value *args, Frame *frame){
    // Create a new Frame f whose parent Frame is frame.
    Frame *f = newFrame(frame);
    
    // If the list of bindings is not a nested list, error.
    if (args->type != CONS_TYPE || 
        car(args)->type != CONS_TYPE || 
        car(car(args))->type != CONS_TYPE) {
        printf("Error: list of bindings for let does not contain a nested list\n");
        texit(EXIT_FAILURE);
    }
    
    // Isolate list of bindings to make
    Value *toAssign = car(args);
    
    // For each binding, add binding to frame f
    while (toAssign->type != NULL_TYPE) {
        Value *cur = car(toAssign);
        
        //make sure binding has 1 variable name and 1 value
        if(cdr(cur)->type == NULL_TYPE || cdr(cdr(cur))->type != NULL_TYPE){
            printf("Error: \"let\" statement does not bind variables correctly.\n");
            texit(EXIT_FAILURE);
        }
        
        // Let vali be the result of evaluating cur value in 
        // Frame frame (environment let statement is in--parent to new frame f).
        Value *vali = eval(car(cdr(cur)), frame);
        
        // Create new binding (cons cell) that includes both the variable (in car) 
        // and result of evaluation of value (in cdr)
        Value *binding = makeNull();
        binding = cons(vali, binding);
        binding = cons(car(cur), binding);
        
        // Add this new binding to f->bindings
        f->bindings = cons(binding, f->bindings);
        
        toAssign = cdr(toAssign);
    }
    
    // Evaluate body in Frame f and return the result.
    // There should only be one arg after the bindings
    // but if there are more, go to the last one (like Racket does).
    // If there is no body, error.
    if(cdr(args)->type == NULL_TYPE){
        printf("Error: \"let\" statement is not formatted properly.\n");
        texit(EXIT_FAILURE);
    }
    
    // Unwrap extra cons cells to get to actual let body and return.
    Value *toReturn = NULL;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(cdr(args));
        while(cdr(curr)->type != NULL_TYPE){
            curr = cdr(curr);
        }
        toReturn = eval(car(curr), f);
    } else {
        toReturn = eval(car(cdr(args)), f);
    }
    return toReturn;
}

/*
 * Evaluates a let* expression with arguments args in environment frame.
 */
Value *evalLetStar(Value *args, Frame *frame){
    //instead of adding each binding to a single frame,
    //create a new frame for each new binding
    
    // Create a new Frame curFrame whose parent Frame is frame.
    Frame *curFrame = newFrame(frame);
    
    //create a holder pointer for the parent frame
    Frame *parentFrame = frame;
    
    // If the list of bindings is not a nested list, error.
    if (args->type != CONS_TYPE || 
        car(args)->type != CONS_TYPE || 
        car(car(args))->type != CONS_TYPE) {
        printf("Error: list of bindings for let does not contain a nested list\n");
        texit(EXIT_FAILURE);
    }
    
    // Isolate list of bindings to make
    Value *toAssign = car(args);
    
    
    // For each binding, add it to the current frame, then create a new frame
    //for the next binding
    while (toAssign->type != NULL_TYPE) {
        
        Value *cur = car(toAssign);
        
        //make sure binding has 1 variable name and 1 value
        if(cdr(cur)->type == NULL_TYPE || cdr(cdr(cur))->type != NULL_TYPE){
            printf("Error: \"let\" statement does not bind variables correctly.\n");
            texit(EXIT_FAILURE);
        }
        
        // Let vali be the result of evaluating cur value in 
        // Frame frame (environment let statement is in--parent to new frame f).
        Value *vali = eval(car(cdr(cur)), curFrame);
        
        // Create new binding (cons cell) that includes both the variable (in car) 
        // and result of evaluation of value (in cdr)
        Value *binding = makeNull();
        binding = cons(vali, binding);
        binding = cons(car(cur), binding);
        
        // Add this new binding to f->bindings
        curFrame->bindings = cons(binding, curFrame->bindings);
        
        //set the parent frame to be the current frame,
        //then create a new frame for the next binding
        parentFrame = curFrame;
        curFrame = newFrame(parentFrame);
        toAssign = cdr(toAssign);
    }
    
    // Evaluate body in Frame curFrame and return the result.
    // There should only be one arg after the bindings
    // but if there are more, go to the last one (like Racket does).
    // If there is no body, error.
    if(cdr(args)->type == NULL_TYPE){
        printf("Error: \"let\" statement is not formatted properly.\n");
        texit(EXIT_FAILURE);
    }
    
    // Unwrap extra cons cells to get to actual let body and return.
    Value *toReturn = NULL;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(cdr(args));
        while(cdr(curr)->type != NULL_TYPE){
            curr = cdr(curr);
        }
        toReturn = eval(car(curr), curFrame);
    } else {
        toReturn = eval(car(cdr(args)), curFrame);
    }
    return toReturn;
}

/*
 * Evaluates a let expression with arguments args in environment frame.
 * Similar to let, but evaluates everything in parent frame
 */
Value *evalLetRec(Value *args, Frame *frame){
    
    // Create a new Frame curFrame whose parent Frame is frame.
    Frame *curFrame = newFrame(frame);
    
    //create a holder pointer for the parent frame
    Frame *parentFrame = frame;
    
    // If the list of bindings is not a nested list, error.
    if (args->type != CONS_TYPE || 
        car(args)->type != CONS_TYPE || 
        car(car(args))->type != CONS_TYPE) {
        printf("Error: list of bindings for let does not contain a nested list\n");
        texit(EXIT_FAILURE);
    }
    
    // Isolate list of bindings to make
    Value *toAssign = car(args);
    
    
    // For each binding, add it to the current frame, then create a new frame
    //for the next binding
    while (toAssign->type != NULL_TYPE) {
        
        Value *cur = car(toAssign);
        
        //make sure binding has 1 variable name and 1 value
        if(cdr(cur)->type == NULL_TYPE || cdr(cdr(cur))->type != NULL_TYPE){
            printf("Error: \"let\" statement does not bind variables correctly.\n");
            texit(EXIT_FAILURE);
        }
        
        // Let vali be the result of evaluating cur value in 
        // Frame frame (environment let statement is in--parent to new frame f).
        
        //TODO: what to do about two symbols defined as each other?
        Value *vali = eval(car(cdr(cur)), curFrame);
        
        // Create new binding (cons cell) that includes both the variable (in car) 
        // and result of evaluation of value (in cdr)
        Value *binding = makeNull();
        binding = cons(vali, binding);
        binding = cons(car(cur), binding);
        
        // Add this new binding to f->bindings
        frame->bindings = cons(binding, frame->bindings);
        
        //set the parent frame to be the current frame,
        //then create a new frame for the next binding
        parentFrame = curFrame;
        curFrame = newFrame(parentFrame);
        toAssign = cdr(toAssign);
    }
    
    // Evaluate body in Frame curFrame and return the result.
    // There should only be one arg after the bindings
    // but if there are more, go to the last one (like Racket does).
    // If there is no body, error.
    if(cdr(args)->type == NULL_TYPE){
        printf("Error: \"let\" statement is not formatted properly.\n");
        texit(EXIT_FAILURE);
    }
    
    // Unwrap extra cons cells to get to actual let body and return.
    Value *toReturn = NULL;
    if (cdr(cdr(args))->type == CONS_TYPE) {
        Value *curr = cdr(cdr(args));
        while(cdr(curr)->type != NULL_TYPE){
            curr = cdr(curr);
        }
        toReturn = eval(car(curr), curFrame);
    } else {
        toReturn = eval(car(cdr(args)), curFrame);
    }
    return toReturn;
}

/*
 * Check to make sure that quote has the proper number of arguments.
 * If it does, return the quoted list.
 */
Value *evalQuote(Value *tree) {
    Value *args = talloc(sizeof(Value));
    args = cdr(tree);
    // Check that quote has exactly one argument 
    if (args->type == NULL_TYPE) {
        printf("Error: \"quote\" not given any arguments\n");
        texit(EXIT_FAILURE);
    }
    if (cdr(args)->type != NULL_TYPE) {
        printf("Error: \"quote\" given too many arguments.\n");
        texit(EXIT_FAILURE);
    }
    // Return the whole tree (including "quote").
    return car(args);
}

/*
 * Evaluates an "if" expression with arguments args in environment frame.
 */
Value *evalIf(Value *args, Frame *frame){
    // Make sure size of args is 3
    Value *cur = args;
    int count = 0;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    if (count != 3) {
        printf("Error: \"if\" statement does not contain three arguments.\n");
        texit(EXIT_FAILURE);
    }
    
    // See if condition is true or false.
    Value *condition = car(args);
    Value *truthValue = eval(condition, frame);
    // Check that condition is a boolean.
    if (truthValue->type != BOOL_TYPE) {
        printf("Error: \"if\" condition does not evaluate to boolean.\n");
        texit(EXIT_FAILURE);
    }
    
    // If true, evaluate second element in args.
    if (!strcmp(truthValue->s, "#t")) {
        return eval(car(cdr(args)), frame);
    }
    
    // If false, evaluate third element in args.
    else {
        return eval(car(cdr(cdr(args))), frame);
    }
}

/*
 * Evaluate a define statement with arguments args and environment frame.
 * Add definition to bindings in frame, return a void value.
 */
Value *evalDefine(Value *args, Frame *frame) {
    // Make sure size of args is 2
    Value *cur = args;
    int count = 0;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    if (count != 2) {
        printf("Error: \"define\" statement does not contain two arguments.\n");
        texit(EXIT_FAILURE);
    }
    
    // Let vali be the result of evaluating value in cur in frame frame.
    Value *vali = eval(car(cdr(args)), frame);
    
    // Create new binding that includes both the variable 
    // and result of evaluation of value.
    Value *binding = makeNull();
    binding = cons(vali, binding);
    binding = cons(car(args), binding);
    
    // Add this binding to frame->bindings
    frame->bindings = cons(binding, frame->bindings);
    
    Value* toReturn = makeNull();
    toReturn->type = VOID_TYPE;
    return toReturn;
}

/*
 * Evaluate a lambda statement with arguments args and environment frame.
 * Add definition to bindings in new frame with current frame as parent,
 * return a closure contain the function.
 */
Value *evalLambda(Value *args, Frame *frame) {
    // Make sure size of args is 2
    Value *cur = args;
    int count = 0;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    if (count < 2) {
        printf("Error: \"lambda\" statement does not contain one or more arguments.\n");
        texit(EXIT_FAILURE);
    }
    
    // Make a new closure that contains the names of the 
    // parameters for the function, the function code, and the environment.
    Value *closure = makeNull();
    closure->type = CLOSURE_TYPE;
    
    closure->cl.paramNames = car(args);
    closure->cl.functionCode = cdr(args);
    closure->cl.frame = frame;
    
    return closure;
}

/*
 * Evaluate a begin statement with arguments args and environment frame.
 * Evaluates all arguments, returns result of last one.
 */
Value *evalBegin(Value *args, Frame *frame) {
    // Make sure size of args is 2
    Value *cur = args;
    int count = 0;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    if (count < 1) {
        printf("Error: \"begin\" statement does not contain two arguments.\n");
        texit(EXIT_FAILURE);
    }
    
    //eval each statement in the function code
    Value *commandList = args;
    cur = car(commandList);
    while(cur->type != NULL_TYPE){
        eval(cur, frame);
        commandList = cdr(commandList);
        if(commandList->type != NULL_TYPE){
            cur = car(commandList);
        }
        else{
            break;
        }
    }
    
    //get the last thing in the list of things that happen in the closure and return that
    Value *lastCommand = car(reverse(args));
    
    return eval(lastCommand, frame);
}

/*
 * Evaluate a set statement with arguments args and environment frame.
 * Add definition to bindings in frame to overwrite old definitions, return a void value.
 */
Value *evalSet(Value *args, Frame *frame) {
    //instead of creating a new binding,
    //look for the binding for this symbol and change it
    
    
    // Make sure size of args is 2
    Value *cur = args;
    int count = 0;
    while (cur->type != NULL_TYPE) {
        count++;
        cur = cdr(cur);
    }
    if (count != 2) {
        printf("Error: \"set!\" statement does not contain two arguments.\n");
        texit(EXIT_FAILURE);
    }
    
    // Let vali be the result of evaluating value in cur in frame frame.
    Value *vali = eval(car(cdr(args)), frame);
    
    Value *symbolToChange = car(args);
    
    //find the appropriate binding in frame/parent frames
    //iterate through the bindings in frame->bindings
    Frame *curFrame = frame;
    int foundMatch = 0;
     while(curFrame != NULL){
        Value *bindingList = curFrame->bindings;
        Value *curBinding = car(bindingList);
        //check all levels of bindings

        while(curBinding->type != NULL_TYPE){
            if(!strcmp(car(curBinding)->s, symbolToChange->s)){
                //change binding value by creating new one to replace
                
                foundMatch = 1;
                Value *newBinding = makeNull();
                newBinding = cons(vali, newBinding);
                newBinding = cons(car(curBinding), newBinding);

                //make the pointer to the old binding now point to the new binding
                curBinding = newBinding;
                curFrame->bindings = cons(curBinding, curFrame->bindings);
            }
            bindingList = cdr(bindingList);
            if(bindingList->type != NULL_TYPE){
                curBinding = car(bindingList);
            }
            else{
                break;
            }
        }
        curFrame = curFrame->parent;
    }
    
    if(foundMatch == 0){
        printf("Error: \"set!\" must modify an existing symbol.\n");
        texit(EXIT_FAILURE);
    }
    
    Value* toReturn = makeNull();
    toReturn->type = VOID_TYPE;
    return toReturn;
}

/*
 * Evaluates an "and" expression, with arguments args, with frame as enviroment.
 * Checks if all arguments are true, short circuits when it finds a false expression.
 */
Value *evalAnd(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        Value *curEvaled = eval(car(current), frame);
        // Returns false as soon as it finds an expression that evaluates to false.
        if (curEvaled->type == BOOL_TYPE) {
            if (!strcmp(curEvaled->s, "#f")) {
                return curEvaled;
            }
        }
        // Error if there are any non-boolean arguments.
        else {
            printf("Error: \"and\" cannot handle non-boolean arguments.\n");
            texit(EXIT_FAILURE);
        }
        current = cdr(current);
    }
    // If we have reached the end of the given args without finding anything false,
    // all args are true; return true.
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->s = "#t";
    return result;
}

/*
 * Evaluates an "or" expression, with arguments args, with frame as enviroment.
 * Checks if any arguments are true, short circuits when it finds a true expression.
 */
Value *evalOr(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        Value *curEvaled = eval(car(current), frame);
        // Returns true as soon as it finds an expression that evaluates to true.
        if (curEvaled->type == BOOL_TYPE) {
            if (!strcmp(curEvaled->s, "#t")) {
                return curEvaled;
            }
        }
        // Error if there are any non-boolean arguments.
        else {
            printf("Error: \"or\" cannot handle non-boolean arguments.\n");
            texit(EXIT_FAILURE);
        }
        current = cdr(current);
    }
    // If we have reached the end of the given args without finding anything true,
    // all args are false; return false.
    Value *result = makeNull();
    result->type = BOOL_TYPE;
    result->s = "#f";
    return result;
}

/*
 * Evaluates a "cond" expression, with arguments args, with frame as enviroment.
 * Return the evaled body of the first true condition, or the default case else.
 * If there are no true conditions, return VOID_TYPE.
 */
Value *evalCond(Value *args, Frame *frame) {
    Value *current = args;
    while (current->type != NULL_TYPE) {
        
        // Check current is a nested cons type
        if (current->type != CONS_TYPE) {
            printf("Error: \"cond\" statement not formatted correctly.\n");
            texit(EXIT_FAILURE);
        }
        
        Value *curExp = car(current);
        
        if (curExp->type != CONS_TYPE) {
            printf("Error: \"cond\" statement not formatted correctly.\n");
            texit(EXIT_FAILURE);
        }
        
        Value *condition = car(curExp);
        Value *body = cdr(curExp);
        
        // Check length of body (there must be exactly one expression in a cond body)
        if (body->type != CONS_TYPE) {
            printf("Error: \"cond\" clause does not have a body.\n");
            texit(EXIT_FAILURE);
        }
        if (cdr(body)->type != NULL_TYPE) {
            printf("Error: \"cond\" body given too many arguments.\n");
            texit(EXIT_FAILURE);
        }
        
        // default "else" case
        if (condition->type == SYMBOL_TYPE &&
            !strcmp(condition->s, "else")) {
            return eval(car(body), frame);
        }
        
        // If this is not the else case, evaluate the condition.
        condition = eval(condition, frame);
        
        // If the condition is not the else case, it must be a boolean.
        if (condition->type != BOOL_TYPE) {
            printf("Error: \"cond\" condition does not evaluate to boolean.\n");
            texit(EXIT_FAILURE);
        }
        
        // The first time we see a condition evaluate to true, evaluate and 
        // return its body. Don't evaluate the other expressions or conditions.
        if (!strcmp(condition->s, "#t")) {
            return eval(car(body), frame);
        }
        current = cdr(current);
    }
    // If we have reached the end of args without returning, there are no true cases.
    // Return VOID_TYPE.
    Value *result = makeNull();
    result->type = VOID_TYPE;
    return result;
}



/*****************************/
/*** Functions and Symbols ***/
/*****************************/


/*
 * Apply the given function closure to the given arguments args.
 */
Value *apply(Value *function, Value *args) {
    Frame *f = newFrame(function->cl.frame);
    
    // Isolate list of bindings to make
    Value *formalParams = function->cl.paramNames;
    Value *actualParams = args;
    // For each binding, add binding to frame f
    while (formalParams->type != NULL_TYPE) {
        // If actualParams is null, error (not enough actual params)
        if (actualParams->type == NULL_TYPE){
            printf("Error: function given too few arguments. \n");
            texit(EXIT_FAILURE);
        }
        Value *curFormal = car(formalParams);
        Value *curActual = car(actualParams);

        // Let vali be the result of evaluating value in cur in parent frame.
        Value *vali = eval(curActual, f->parent);
        // Create new binding that includes the formal param and the corresponding actual param
        Value *binding = makeNull();
        binding = cons(vali, binding);
        binding = cons(curFormal, binding);
        // Add this binding to f->bindings
        f->bindings = cons(binding, f->bindings);
        
        formalParams = cdr(formalParams);
        actualParams = cdr(actualParams);
    }
    // If actualParams is not null, error (too many actual params)
    if (actualParams->type != NULL_TYPE){
        printf("Error: function given too many arguments. \n");
        texit(EXIT_FAILURE);
        }

    //eval each statement in the function code
    Value *commandList = function->cl.functionCode;
    Value *cur = car(commandList);
    while(cur->type != NULL_TYPE){
        //skip begin statements, since lambda already has an implicit begin statement
        if(cur->type == CONS_TYPE && car(cur)->type == SYMBOL_TYPE && (!strcmp(car(cur)->s, "begin"))){
            commandList = cdr(commandList);
        }
        else{
            eval(cur, f);
            commandList = cdr(commandList);
        }
        if(commandList->type != NULL_TYPE){
            cur = car(commandList);
        }
        else{
            break;
        }
    }
    
    //get the last thing in the list of things that happen in the closure and return that
    Value *lastCommand = car(reverse(function->cl.functionCode));
    
    return eval(lastCommand, f);
    
}

/*
 * Looks up the value of a variable using current and parent frames.
 */
Value *lookUpSymbol(Value *tree, Frame *frame){
    // Loop through all bindings in current frame
    Value *curBindings = frame->bindings;
    while (curBindings->type != NULL_TYPE) {
        // If we've found a match, return the result
        if(strcmp(tree->s, car(car(curBindings))->s) == 0){
            if (cdr(car(curBindings))->type == CONS_TYPE){
                return car(cdr(car(curBindings)));
            }
            else {
                return cdr(car(curBindings));
            }
        }
        curBindings = cdr(curBindings);
    }
    // Print error if variable is not bound in current or parent frames
    if(frame->parent == NULL){
        printf("Error 404: variable not found: ");
        display(tree);
        printf("\n");
        texit(EXIT_FAILURE); 
    }
    // Recurse on parent frame
    return lookUpSymbol(tree, frame->parent);
    
}
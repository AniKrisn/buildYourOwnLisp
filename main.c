#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char *readline(char *prompt) {
    fputs(prompt, stdout);
    fgets(buffer, sizeof(buffer), stdin);
    // fgets reads in a string from the file stream and stores it in the buffer array
    char *cpy = malloc(strlen(buffer+1));
    // we use strlen(buffer) rather than sizeof(buffer) here because the latter would allocate the entire length of the array and the string is likely far smaller
    strcpy(cpy, buffer);
    // destination is the first arg. Same with fgets above
    cpy[strlen(cpy)-1] = '\0';
    // fgets, like puts, appends a newline character at the end of the string when it stores it in the buffer array. We replace this newline character with a null terminator
    // remember that e.g strlen of "hello/n" is 6, and the index of '\n' is 5. That's why strlen-1
    return cpy;
}


//??
void add_history(char *unused) {}


#else 
#include <editline/readline.h>
#include <editline/history.h>
#endif



// def "lisp value" -- 
typedef struct {
    int type;
    long num;
    // err and symbol types have string data
    char *err;
    char *sym;
    // count and point to a list of "lval*"
    int count;
    struct lval **cell;
} lval;

// possible lval types
enum { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };
// possible err types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

// construct a pointer to a new num lval
lval *lval_num (long x) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
}
// construct a pointer to a new err lval
lval *lval_err(char *m) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v; 
}
// construct a pointer to a new sym lval
lval *lval_sym(char *s) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}
// a pointer to a new empty sexpr lval
lval *lval_sexpr(void) {
    lval *v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}



/* use operator string to see which operation to perform */
lval eval_op(lval x, char* op, lval y) {

    // if either operands are errs return err
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }
    
    if (strcmp(op, "+") == 0) {return lval_num(x.num + y.num);}
    if (strcmp(op, "-") == 0) {return lval_num(x.num - y.num);}
    if (strcmp(op, "*") == 0) {return lval_num(x.num * y.num);}
    if (strcmp(op, "/") == 0) {
        // if second operand is 0 return corresponding err
        return y.num == 0
        ? lval_err(LERR_DIV_ZERO)
        : lval_num(x.num / y.num);
        
    }
    
    return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t *t) {

    // if tagged as number return directly
    if (strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    // since the operator is always the second child:
    char *op = t->children[1]->contents;    

    // store the third child in 'x'
    lval x = eval(t->children[2]);

    // recursively iterate through remaining children and combine. this is really cool
    int i = 3;
    while(strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}


// print an lval
void lval_print(lval v) {
    switch (v.type) {
        case LVAL_NUM: printf("%li", v.num); break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by Zero!");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
        break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }


int main (int argc, char **argv) {

    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Sexpr = mpc_new("sexpr");
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number   : /-?[0-9]+/ ;                             \
        symbol   : '+' | '-' | '*' | '/' ;                  \
        sexpr    : '(' <expr>* ')' ;                        \
        expr     : <number> | '(' <operator> <expr>+ ')' ;  \
        lispy    : /^/ <operator> <expr>+ /$/ ;             \
    ",
    Number, Symbol, Sexpr, Expr, Lispy);


    puts("Lispy Version 0.1");
    puts("Press Ctrl+C to escape\n");  
    
    while (1) {

        char *input = readline("lispy> ");

        // readline history is stored separately, primarily in the heap. Calling free() doesn't affect this; there are separate functions to read/write the readline history
        add_history(input);

        // pass user input
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {

            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        // if successful, eval & print, else error

        free(input);
    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
    // undefine and delete the parsers

    return 0;
}
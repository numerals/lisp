#include <stdio.h>
#include <stdlib.h>

// Helps in making REPL
#include <editline/readline.h>
#include <editline/history.h>

// Including MPC lib
#include "mpc.h"

typedef struct lval {
    int type;
    long number;
    int err;
} lval;

enum {LVAL_NUM, LVAL_ERR};
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

lval eval_op(lval x, char* op, lval y);
lval eval(mpc_ast_t* t);
void lval_println(lval v);
lval lval_num(long x);
lval lval_err(int x);
void lval_print(lval v);

int main(int argc, char** argv) {


    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT, 
            "                                                   \
            number   : /-?[0-9]+/ ;                             \
            operator : '+' | '-' | '*' | '/' ;                  \
            expr     : <number> | '(' <operator> <expr>+ ')' ;  \
            lispy    : /^/ <operator> <expr>+ /$/ ;             \
            ",
            Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.0.1\n");
    puts("Press Ctrl+c to exit\n");

    while(1) {

        char* input = readline("MyLisp>> ");

        add_history(input);

        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the Result */
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);

        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }

    mpc_cleanup(4, Number, Operator, Expr, Lispy);

    return 0;
}

lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.number = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

void lval_print(lval v) {

    switch(v.type) {
        case LVAL_NUM:
            printf("%ld", v.number);
            break;
        case LVAL_ERR:
            if(v.err == LERR_DIV_ZERO) {
                printf("Error: Division By Zero");
            }
            if(v.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number");
            }
            if(v.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator");
            }
            break;
    }

}

void lval_println(lval v) {
    lval_print(v);
    putchar('\n');
}

lval eval(mpc_ast_t* t) {

    if(strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    char* op = t->children[1]->contents;

    lval x = eval(t->children[2]);

    int i = 3;

    while(strstr(t->children[i]->tag, "expr")) {

        x = eval_op(x, op, eval(t->children[i]));
        ++i;
    }

    return x;
}

lval eval_op(lval x, char* op, lval y) {

    if(x.type == LVAL_ERR) {
        return x;
    }
    if(y.type == LVAL_ERR) {
        return y;
    }
    if(!strcmp(op, "+")) {
        return lval_num(x.number + y.number);
    }
    else if(!strcmp(op, "-")) {
        return lval_num(x.number - y.number);
    }
    else if(!strcmp(op, "*")) {
        return lval_num(x.number * y.number);
    }
    else if(!strcmp(op, "/")) {
        return y.number == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.number / y.number);
    }

    return lval_err(LERR_BAD_OP);

}

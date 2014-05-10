#include <stdio.h>
#include <stdlib.h>

// Helps in making REPL
#include <editline/readline.h>
#include <editline/history.h>

// Including MPC lib
#include "mpc.h"

long eval(mpc_ast_t* t);
long eval_op(long x, char* op, long y);

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
            lispy    : /^/ <operator> <expr>+ /$/ ;       \
            ",
            Number, Operator, Expr, Lispy);

    puts("Lispy Version 0.0.1\n");
    puts("Press Ctrl+c to exit\n");

    while(1) {

        char* input = readline("MyLisp>> ");

        add_history(input);

        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            long result = eval(r.output);
            printf("%ld\n", result);
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

long eval(mpc_ast_t* t) {

    if(strstr(t->tag, "number")) {

        return atoi(t->contents);

    }

    char* op = t->children[1]->contents;

    long x = eval(t->children[2]);

    int i = 3;

    while(strstr(t->children[i]->tag, "expr")) {

        x = eval_op(x, op, eval(t->children[i]));
        ++i;
    }

    return x;
}

long eval_op(long x, char* op, long y) {

    if(!strcmp(op, "+")) {
        return x + y;
    }
    else if(!strcmp(op, "-")) {
        return x - y;
    }
    else if(!strcmp(op, "*")) {
        return x * y;
    }
    else if(!strcmp(op, "/")) {
        return x / y;
    }
    return 0;

}

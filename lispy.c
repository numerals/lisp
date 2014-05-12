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

    char* err;
    char* sym;

    int count;
    struct lval** cell;

}lval;

enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

lval* lval_num(long x);
lval* lval_err(char* s);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
void lval_del(lval* v);
lval* lval_add(lval* v, lval* x);
lval* lval_read_num(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
void lval_print_expr(lval* v, char open, char close);
void lval_println(lval* v);
void lval_print(lval* v);
//lval eval(mpc_ast_t* t);
//lval eval_op(lval x, char* op, lval y);

int main(int argc, char** argv) {

    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    mpca_lang(MPCA_LANG_DEFAULT, 
            "                                                   \
            number   : /-?[0-9]+/ ;                             \
            symbol   : '+' | '-' | '*' | '/' ;                  \
            sexpr    : '(' <expr>* ')' ;                        \
            expr     : <number> | <symbol> | <sexpr> ;          \
            lispy    : /^/ <expr>+ /$/ ;             \
            ",
            Number, Symbol, Sexpr, Expr, Lispy);

    puts("Lispy Version 0.0.1\n");
    puts("Press Ctrl+c to exit\n");

    while(1) {

        char* input = readline("MyLisp>> ");

        add_history(input);

        mpc_result_t r;

        if(mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the Result */
            lval* result = lval_read(r.output);
            lval_println(result);
            lval_del(result);

        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }

    mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

    return 0;
}

lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->number = x;
    return v;
}

lval* lval_err(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(s) + 1);
    strcpy(v->err, s);
    return v;
}

lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}

lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

void lval_del(lval* v) {

    switch(v->type) {
        case LVAL_NUM:
            break;
        case LVAL_ERR:
            free(v->err);
            break;
        case LVAL_SYM:
            free(v->sym);
            break;
        case LVAL_SEXPR:

            for(int i = 0; i < v->count; i++) {
                lval_del(v->cell[i]);
            }

            free(v->cell);
            break;
    }

    free(v);

}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count - 1] = x;
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err("Invalid Number");
}

lval* lval_read(mpc_ast_t* t) {
    if(strstr(t->tag, "number")) {
        return lval_read_num(t);
    }
    if(strstr(t->tag, "symbol")) {
        return lval_sym(t->contents);
    }

    lval* x = NULL;
    if(strcmp(t->tag, ">") == 0) {
        x = lval_sexpr();
    }
    if(strstr(t->tag, "sexpr")) {
        x = lval_sexpr();
    }

    for(int i = 0; i < t->children_num; i++) {
        if(strcmp(t->children[i]->contents, "(") == 0) {
            continue;
        }
        if(strcmp(t->children[i]->contents, ")") == 0) {
            continue;
        }
        if(strcmp(t->children[i]->contents, "{") == 0) {
            continue;
        }
        if(strcmp(t->children[i]->contents, "}") == 0) {
            continue;
        }
        if(strcmp(t->children[i]->tag, "regex") == 0) {
            continue;
        }
        x = lval_add(x, lval_read(t->children[i]));
    }
    return x;
}

void lval_print_expr(lval* v, char open, char close) {

    putchar (open);

    for(int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);

        if(i != v->count - 1) {
            putchar(' ');
        }

    }
    putchar(close);

}

void lval_println(lval* v) {
    lval_print(v);
    putchar('\n');
}

void lval_print(lval* v) {

    switch(v->type) {
        case LVAL_NUM:
            printf("%ld", v->number);
            break;
        case LVAL_ERR:
            printf("Error: %s", v->err);
            break;
        case LVAL_SYM:
            printf("%s", v->sym);
            break;
        case LVAL_SEXPR:
            lval_print_expr(v, '(', ')');
            break;
    }
}

/*
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
   */

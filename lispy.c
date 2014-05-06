#include <stdio.h>
#include <stdlib.h>

// Helps in making REPL
#include <editline/readline.h>
#include <editline/history.h>

int main() {

    puts("Lispy Version 0.0.1\n");
    puts("Press Ctrl+c to exit\n");

    while(1) {

        char* input = readline("MyLisp>> ");

        add_history(input);

        puts("yes");

        free(input);

    }

    return 0;
}


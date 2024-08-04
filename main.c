#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>


void add_history(char *unused) {}


#else 
#include <editline/readline.h>
#include <editline/history.h>
#endif



int main (int argc, char **argv) {

    char* input;

    while (input = readline("lispy> ") != NULL) {
        add_history(input);
    }

}
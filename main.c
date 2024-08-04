#include <stdio.h>
#include <stdlib.h>

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


void add_history(char *unused) {}


#else 
#include <editline/readline.h>
#include <editline/history.h>
#endif


int main (int argc, char **argv) {

    puts("Lispy Version 0.1");
    puts("Press Ctrl+C to escape\n");

    while (1) {
        char *input = readline("lispy> ");
        add_history(input);
        // readline history is stored separately, primarily in the heap. Calling free() doesn't affect this; there are separate functions to read/write the readline history

        printf("No you're a %s!\n", input);
        free(input);
    }

    return 0;
}
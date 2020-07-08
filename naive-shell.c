#include "naive-shell.h"

int main() {
    char* line = NULL;
    while ((line = nsh_readline()) != NULL) {
        Log("%s", line);
    }

    return 0;
}

/* get a line from a user with editing */
char* nsh_readline() {
    char* line = NULL;
    /* The line returned is allocated with malloc(3); the caller must free it when finished.  */
    if (line) {
        free(line);
        line = NULL;
    }

    line = readline("nsh > ");
    /* 只有当键入EOF,并且line还是空的时候才返回NULL */
    if (line != NULL && *line) {
        add_history(line); /* support ↑ ↓ ← → */
    }
    return line;
}
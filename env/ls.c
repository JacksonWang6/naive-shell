#include "ls.h"

int main(int argc, char* argv[]) {
    success_g = true;
    /* test argc & argv */
    Log("%d", argc);
    for (int i = 0; i < argc; i++) {
        Log("argv[%d]: %s", i, argv[i]);
    }

    /* parse arguments */
    char* path = parse_arguments(argc, argv);
    if (!success_g) return 0;
    if (path != NULL) {
        /* parse path */
        Log("path: %s", path);
    }
}

/* return value: if argu contions path, then return path, otherwise return null */
char* parse_arguments(int argc, char* argv[]) {
    bool ispath = false;
    char* path = NULL;
    for (int i = 1; i < argc; i++) {
        Assert(argv[i] != NULL, "error");
        /* if start with '-', it maybe argument, else it maybe path */
        if (argv[i][0] == '-') {
            int j;
            for (j = 0; j < NR_OPTIONS; j++) {
                if (strcmp(argv[i], options[j].name) == 0 || strcmp(argv[i], options[j].full_name) == 0) {
                    *(options[j].target) = true;
                    break;
                }
            }
            if (j == NR_OPTIONS) {
                printf("Unknown argument: %s\n", argv[i]);
                success_g = false;
                return NULL;
            }
        } else {
            if (ispath == false) {
                ispath = true;
                path = (char*) malloc (sizeof(char) * strlen(argv[i]) * 2);
                path = argv[i];
            } else {
                printf("[error] Usage: ls [options] [path]\npath only 0 or 1\n");
                success_g = false;
                return NULL;
            }
        }
    }
    return path;
}
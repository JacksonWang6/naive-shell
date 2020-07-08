#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
/* use readline */
#include <readline/readline.h>
#include <readline/history.h>
#include "debug.h"

// int cmd_echo(char* args);
// int cmd_exit(char* args);
// int cmd_cd(char* args);
// int cmd_pwd(char* args);

// struct {
//     char* name;
//     char* str;
//     int (*handler) (char*);
// } cmd_table [] = {
//     {"echo", "display a line of text", cmd_echo},
//     {"exit", "cause normal process termination", cmd_exit},
//     {"cd", "change directory", cmd_cd},
//     {"pwd", "print name of current/working directory", cmd_pwd},
// };

char* nsh_readline();
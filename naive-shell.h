#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
/* use readline */
#include <readline/readline.h>
#include <readline/history.h>
/* achieve signal mechanism */
#include <signal.h>
/* for use strtok */
#include <string.h>
/* for use fork */
#include <sys/types.h>
#include <unistd.h> /* chdir, getcwd */
/* for use wait */
#include <sys/wait.h>
/* for use open */
#include <sys/stat.h>
#include <fcntl.h>
#include "debug.h"

#define BUF_SIZE 512

int cmd_test(char* args);
int cmd_help(char* args);
int cmd_echo(char* args);
int cmd_exit(char* args);
int cmd_cd(char* args);
int cmd_pwd(char* args);

struct {
    char* name;
    char* str;
    int (*handler) (char*);
} cmd_table [] = {
    {"help", "Usage: cmd [space][arguments]\nand you can use below cmds:\n", cmd_help},
    {"test", "test my program", cmd_test},
    {"echo", "display a line of text", cmd_echo},
    {"exit", "cause normal process termination", cmd_exit},
    {"cd", "change directory", cmd_cd},
    {"pwd", "print name of current/working directory", cmd_pwd},
};
#define CMD_NUM (sizeof(cmd_table) / sizeof(cmd_table[0]))

char* nsh_readline();
void int_handler(int signum);
void exec_cmd(char* line);
char* find_special(char* line);
void deal_special(char* line, char* pos);
void special_exec_cmd(char* line);

int pid = -1;
char special_tokens[] = {'>', '<', '|'};  /* redirection and pipe */
#define TOKENS_NUM (sizeof(special_tokens) / sizeof(special_tokens[0]))
bool exist_special_tokens = false;
char* str_end;
char* special_args;
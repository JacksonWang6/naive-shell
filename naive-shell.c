#include "naive-shell.h"

int main() {
    /* register handler */
    signal(SIGINT, int_handler);

    char* line = NULL;
    char *str_end = line + strlen(line);
    while ((line = nsh_readline()) != NULL) {
        pid = -1;
        Log("pid: %d", pid);
        Log("%s", line);
        #ifdef TEST
        /* test strtok */
        char* cmd = strtok(line, " ");
        Log("cmd: %s", cmd);
        while (cmd != NULL) {
            cmd = strtok(NULL, " ");
            Log("cmd: %s", cmd);
        }
        #endif
        char* cmd = strtok(line, " ");
        /* empty line */
        if (cmd == NULL) continue;
        /* strtok will set the first delim to '\0', sooo... */
        char* args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }
        /* search cmd and exec it */
        int i;
        for (i = 0; i < CMD_NUM; i++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                int id = fork();
                if (id == 0) {
                    pid = getpid();
                    Log("fork pid: %d", pid);
                    /* special: cd should be exec in parent process */
                    if (strcmp(cmd_table[i].name, "cd") == 0) return 0;
                    cmd_table[i].handler(args);
                    return 0;
                } else {
                    /* wait son */
                    pid = id;
                    wait(NULL);
                    kill(pid, SIGKILL);
                    if (strcmp(cmd_table[i].name, "cd") == 0) {
                        cmd_table[i].handler(args);
                    }
                    break;
                }
            }
        }
        /* not found */
        if (i == CMD_NUM) {
            printf("nsh: command not found: %s\n", cmd);
            printf("please use help to know more.\n");
        }
        fflush(stdout);
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
    /* add green font, looks cool^_^ */
    line = readline( ESCAPE FG_GREEN "nsh > " ESCAPE);
    /* readline  returns  the  text  of the line read.  A blank line returns the empty string.  If EOF is encountered while reading a line, and the
     * line is empty, NULL is returned.  If an EOF is read with a non-empty line, it is treated as a newline.
     */
    if (line != NULL && *line) {
        add_history(line); /* support ↑ ↓ ← → */
    }
    return line;
}

/* ctrl-d 不是发送信号，而是表示一个特殊的二进制值，表示 EOF。 */
/* ctrl + c signal handler */
void int_handler(int signum) {
    Log("sig int, pid: %d", pid);
    if (pid != -1) {
        kill(pid, SIGKILL);
    }
}

int cmd_test(char* args) {
    CLog(FG_RED, "%s", args);
    /* for test ctrl + c */
    while (1) {
        for (volatile int i = 0; i <= 10000; i++) ;
    }
    exit(0);
}

int cmd_help(char* args) {
    for (int i = 0; i < CMD_NUM; i++) {
        printf("%s: %s\n", cmd_table[i].name, cmd_table[i].str);
    }
    return 0;
}

/* shell build-in cmd */
int cmd_echo(char* args) {
    CLog(FG_RED, "%s", args);
    if (args == NULL) printf("\n");
    else printf("%s\n", args);
    return 0;
}

int cmd_exit(char* args) {
    CLog(FG_RED, "%s", args);
    int id = getpid();
    int ppid = getppid();
    kill(ppid, SIGKILL);
    kill(id, SIGKILL);
    exit(0);
}

/* bug: fork出来的进程更改了目录,但是父进程并没有更改 */
int cmd_cd(char* args) {
    CLog(FG_RED, "%s", args);
    int ret = chdir(args);
    if (ret == -1) {
        //error handler
        printf("cd: no such file or dir: %s\n", args);
        return -1;
    }
    return 0;
}

int cmd_pwd(char* args) {
    CLog(FG_RED, "%s", args);
    char path_buf[128];
    char* cur;
    cur = getcwd(path_buf, 128);
    if (cur == NULL) {
        printf("there is some error.\n");
    } else {
        printf("%s\n", cur);
    }
    return 0;
}
#include "naive-shell.h"

int main() {
    /* register handler */
    signal(SIGINT, int_handler);

    char* line = NULL;
    while ((line = nsh_readline()) != NULL) {
        str_end = line + strlen(line);
        pid = -1;
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
        /* 由于题目说了不用考虑过于复杂的边界情况,那么我就不管><|在字符串里面等情况了 */
        char* pos = find_special(line); /* 寻找 <>|, 找到,返回对应位置的指针,否则, 返回NULL */
        if (pos != NULL) {
            deal_special(line, pos); /* if exist <>|, we should special deal */
            continue;
        }
        exec_cmd(line);        
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

void exec_cmd(char* line) {
    char* cmd = strtok(line, " ");
    /* empty line */
    if (cmd == NULL) return;
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
                if (strcmp(cmd_table[i].name, "cd") == 0) exit(0);
                cmd_table[i].handler(args);
                exit(0);
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
    return;
}

char* find_special(char* line) {
    char* pos = NULL;
    int i;
    for (i = 0; i < TOKENS_NUM; i++) {
        pos = strchr(line, special_tokens[i]);
        if (pos != NULL) {
            break;
        }
    }
    return pos;
}

void deal_special(char* line, char* pos) {
    /* They are the left and right of the special token */
    char left[BUF_SIZE] = {0};
    char right[BUF_SIZE] = {0};
    int i = 0;
    while (line[i] != *pos) {
        left[i] = line[i];
        i++;
    }
    left[i] = 0;
    i++;
    while (line[i] == ' ') i++; /* 跳过空格 */
    int k = 0;
    while (line[i+k]) {
        right[k] = line[i+k];
        k++;
    }
    right[k] = 0;
    Log("token: %c", *pos);
    int pipefd[2] = {};
    pid_t id;
    if (pipe(pipefd) == -1) {
        Assert(0, "pipe error");
    }
    switch (*pos) {
        case '>':
            /* open file and dup fd */
            id = fork();
            if (id == 0) {
                pid = getpid();
                Log("ppid: %d", getppid());
                close(pipefd[0]); /* close read */
                dup2(pipefd[1], STDOUT_FILENO);
                CLog(FG_RED, "left: %s, ppid: %d", left, getppid());
                special_exec_cmd(left);
            } else {
                CLog(FG_RED, "run here");
                close(pipefd[1]); /* close write */
                int fd_out = open(right, O_CREAT | O_RDWR, 0777);
                char buf[BUF_SIZE << 4] = {0};
                int cnt = 0;
                while ((cnt = read(pipefd[0], buf, BUF_SIZE<<4)) > 0) {
                    CLog(FG_GREEN, "%s", buf);
                    write(fd_out, buf, cnt);
                } 
                wait(NULL);
                kill(id, SIGKILL);
                close(fd_out);
                if (special_args != NULL) {
                    Assert(special_args != NULL, "null error");
                    cmd_table[4].handler(special_args);
                    free(special_args);
                }
            }
            break;
        case '<':
            id = fork();
            if (id == 0) {
                pid = getpid();
                close(pipefd[1]); /* close write */
                // dup2(pipefd[0], STDIN_FILENO);
                CLog(FG_RED, "left: %s", left);
                /* 拼接args */
                char buf[BUF_SIZE] = {0};
                char tmp_buf[BUF_SIZE] = {0};
                sprintf(buf, "%s ", left);
                int cnt = 0;
                /* 这里的read貌似阻塞了,不知道怎么破... */
                while ((cnt = read(pipefd[0], tmp_buf, BUF_SIZE)) > 0) {
                    Log("buf: %s", buf);
                    strncat(buf, tmp_buf, cnt);
                    Log("buf: %s", buf);
                }
                buf[strlen(buf) - 1] = '\0';
                Log("buf: %s", buf);
                special_exec_cmd(buf);
            } else {
                CLog(FG_RED, "run here");
                close(pipefd[0]); /* close read */
                int fd_in = open(right, O_RDONLY | O_NONBLOCK, 0777);
                char buf[BUF_SIZE] = {0};
                int cnt = 0;
                /* write to pipefd[1] */
                while ((cnt = read(fd_in, buf, BUF_SIZE)) > 0) {
                    write(pipefd[1], buf, cnt);
                    CLog(FG_RED, "%s: %d", buf, cnt);
                }
                wait(NULL);
                close(fd_in);
                kill(id, SIGKILL);
                if (special_args != NULL) {
                    Assert(special_args != NULL, "null error");
                    cmd_table[4].handler(special_args);
                    free(special_args);
                }
            }
            break;
        /* 目前的这几条内建命令根本测试不了管道... */
        case '|':
            /* 先todo一下,明天再做 */
            // TODO();
            id = fork();
            if (id == 0) {
                /* communicate by pipefd2 */
                int pipefd2[2];
                if (pipe(pipefd2) == -1) {
                    Assert(0, "pipe error");
                }
                /* fork again, because we should exec cmd twice */
                int id2 = fork();
                if (id2 == 0) {
                    /* son, close write, and read from parent */
                    close(pipefd2[1]);
                    dup2(pipefd2[0], STDIN_FILENO);
                    exec_cmd(right);
                    exit(0);
                } else {
                    /* parent, close read, and write to son */
                    close(pipefd2[0]);
                    dup2(pipefd2[1], STDOUT_FILENO);
                    exec_cmd(left);
                }
            } else {
                wait(NULL);
                kill(id, SIGKILL);
            }
            break;
        default:
            CLog(FG_RED, "run here");
            break;
    }
}

void special_exec_cmd(char* line) {
    str_end = line + strlen(line);
    CLog(FG_RED, "%s", line);
    char* cmd = strtok(line, " ");
    CLog(FG_RED, "%s", cmd);
    /* empty line */
    if (cmd == NULL) return;
    /* strtok will set the first delim to '\0', sooo... */
    char* args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
        args = NULL;
    }
    if (args == NULL) special_args = NULL;
    /* search cmd and exec it */
    int i;
    for (i = 0; i < CMD_NUM; i++) {
        if (strcmp(cmd, cmd_table[i].name) == 0) {
            /* zsh cd的输出貌似不能重定向,测试过 */
            if (strcmp(cmd_table[i].name, "cd") == 0) {
                /* fix bug: *** buffer overflow detected ***: */
                special_args = (char*) malloc (strlen(args) * sizeof(char) * 2);
                strncpy(special_args, args, strlen(args));
                exit(0);
            }
            cmd_table[i].handler(args);
            exit(0);
        }
    }
    /* not found */
    if (i == CMD_NUM) {
        printf("nsh: command not found: %s\n", cmd);
        printf("please use help to know more.\n");
    }
    fflush(stdout);
    return;
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
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
            /* judge built-in or not */
            for (int ii = 0; ii < NR_BUILT_IN_CMD; ii++) {
                if (strcmp(cmd, built_in_cmd[ii]) == 0) {
                    cmd_table[i].handler(args);
                    break;
                }
            }
            /* 否则在子进程当中执行这一条命令,因为在子进程当中才能杀死 */
            int id = fork();
            if (id == 0) {
                pid = getpid();
                Log("fork pid: %d", pid);
                cmd_table[i].handler(args);
                exit(0);
            } else {
                /* wait son */
                pid = id;
                wait(NULL);
                kill(id, SIGKILL);
                break;
            }
        }
    }
    /* not built-in cmd, try bin */
    if (i == CMD_NUM) {
        if (match_export_cmd(cmd, args) == false) {
            printf("nsh: command not found: %s\n", cmd);
            printf("please use help to know more.\n");
        }
    }
    return;
}

void pipe_exec_cmd(char* line) {
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
            cmd_table[i].handler(args);
            break;
        }
    }
    /* not built-in cmd, try bin */
    if (i == CMD_NUM) {
        if (match_export_cmd(cmd, args) == false) {
            printf("nsh: command not found: %s\n", cmd);
            printf("please use help to know more.\n");
        }
    }
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
                Log("子进程, runhere");
                close(pipefd[1]);
                exit(0);
            } else {
                pid = id;
                CLog(FG_RED, "run here");
                close(pipefd[1]); /* close write */
                int fd_out = open(right, O_CREAT | O_RDWR, 0777);
                char buf[BUF_SIZE << 4] = {0};
                int cnt = 0;
                while ((cnt = read(pipefd[0], buf, BUF_SIZE<<4)) > 0) {
                    CLog(FG_GREEN, "%s", buf);
                    write(fd_out, buf, cnt);
                } 
                Log("重定向>: run here");
                wait(NULL);
                kill(id, SIGKILL);
                close(pipefd[0]);
                close(fd_out);
                if (special_args != NULL) {
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
                dup2(pipefd[0], STDIN_FILENO);
                CLog(FG_RED, "left: %s", left);
                special_exec_cmd(left);
                close(pipefd[0]);
                exit(0);
            } else {
                pid = id;
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
                close(pipefd[1]);
                close(fd_in);
                kill(id, SIGKILL);
                if (special_args != NULL) {
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
                pid = getpid();
                /* communicate by pipefd2 */
                int pipefd2[2];
                if (pipe(pipefd2) == -1) {
                    Assert(0, "pipe error");
                }
                /* fork again, because we should exec cmd twice */
                int id2 = fork();
                if (id2 == 0) {
                    /* son, close read, and write to parent */
                    close(pipefd2[0]);
                    dup2(pipefd2[1], STDOUT_FILENO);
                    close(pipefd2[1]);
                    printf("dup2 dup2\n");
                    Log("left: %s", left);
                    pipe_exec_cmd(left);
                    exit(0);
                } else {
                    /* parent, close write, and read from son */
                    close(pipefd2[1]);
                    dup2(pipefd2[0], STDIN_FILENO);
                    printf("wait\n");
                    close(pipefd2[0]);
                    /* 测试结果显示,这里没有接受到任何stdin的数据... */
                    Log("right: %s", right);
                    int status;
                    waitpid(id2, &status, 0); /* wait son */
                    printf("wait\n");
                    pipe_exec_cmd(right);
                    printf("wait\n");
                    kill(id2, SIGKILL);
                    exit(0);
                }
                exit(0);
            } else {
                wait(NULL);
                pid = id;
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
                return;
            }
            cmd_table[i].handler(args);
            return;
        }
    }
    /* not built-in cmd, try bin */
    if (i == CMD_NUM) {
        if (match_export_cmd(cmd, args) == false) {
            printf("nsh: command not found: %s\n", cmd);
            printf("please use help to know more.\n");
        }
    }
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
    /* for test ctrl + c and pipe, redirection */
    while (1) {
        for (volatile int i = 0; i <= 10000; i++) {
            char buf[BUF_SIZE] = {0};
            scanf("%s", buf);
            if (strlen(buf) != 0) {
                CLog(FG_GREEN, "%s", buf);
                printf("%s", buf);
            }
            // printf("--------------------------\n%s\n---------------------------------\n", buf);
        }
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

int cmd_export(char* args) {
    CLog(FG_RED, "%s", args);
    /* feature: if there are many paths, it should be Separated by ':' */
    char* path = strtok(args, ":");
    while (path != NULL) {
        path_table[path_cnt] = (char*) malloc (sizeof(char) * strlen(args) * 2);
        strcpy(path_table[path_cnt], path);
        Log("path: %s", path);
        path_cnt++;
        path = strtok(NULL, ":");
    }
    return 0;
}

/* return value: match ok or fail */
bool match_export_cmd(char* cmd, char* args) {
    CLog(FG_RED, "%s", cmd);
    CLog(FG_RED, "pathcnt: %d", path_cnt);
    if (path_cnt == 0) return false;
    char buf[BUF_SIZE] = {0};
    int id = fork();
    if (id == -1) Assert(0, "fork error");
    if (id == 0) {
        int i,j;
        char* exec_argv[MAX_ARGV];
        char* arg = strtok(args, " ");
        exec_argv[0] = cmd;
        j = 1;
        while (arg != NULL) {
            exec_argv[j] = arg;
            arg = strtok(NULL, " ");
            j++;
        }
        exec_argv[j] = NULL;
        char* exec_envp[2];
        for (i = 0; i < path_cnt; i++) {
            exec_envp[0] = path_table[i];
            exec_envp[1] = NULL;
            if (path_table[i][strlen(path_table[i])-1] != '/') {
                sprintf(buf, "%s/%s", path_table[i], cmd);
            } else {
                sprintf(buf, "%s%s", path_table[i], cmd);
            }
            if (execve(buf, exec_argv, exec_envp) == -1) continue;
        }
        exit(1);
    }
    /* 这里不知道怎么获取子进程执行成功还是失败的结果,那么就用一个蠢办法吧 */
    /* get the filenames of path */
    DIR* dir;
    struct dirent* ptr;
    Log("cmdname: %s, len: %zu", cmd, strlen(cmd));
    for (int i = 0; i < path_cnt; i++) {
        if ((dir = opendir(path_table[i])) == NULL) {
            Assert(0, "open %s error", path_table[i]);
        }
        while ((ptr = readdir(dir)) != NULL) {
            if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
                continue;
            else if(ptr->d_type == 8) {
                /* file */
                Log("filename: %s, len: %zu", ptr->d_name, strlen(ptr->d_name));
                if ((strncmp(ptr->d_name, cmd, strlen(cmd)) == 0) && (strlen(ptr->d_name) == strlen(cmd))) return true;
	        }
            else continue;
        }
    }
    return false;
}
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>           /* Definition of AT_* constants */
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "../debug.h"

#define BUF_SIZE 128

struct option {
  const char* name;
  const char* full_name;
  bool* target;
};

bool OP_LIST = false;

const struct option options[] = {
    {"-l", "--list", &OP_LIST},
};
const int NR_OPTIONS = (int) sizeof(options) / sizeof(struct option);
bool success_g = true;

/* 函数声明 */
char* parse_arguments(int argc, char* argv[]);
void parse_dir(char* cur_path);

typedef struct file_info {
    char name[BUF_SIZE];    /* 文件名 */
    mode_t mode;            /* 权限 */
    uid_t uid;                /* 所有者 */
    gid_t gid;                /* 群组 */
    off_t size;               /* 大小 */
    struct timespec mtim;   /* 上次修改时间 */
    bool l;
} file_info_t;
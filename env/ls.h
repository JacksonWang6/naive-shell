#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#include "../debug.h"

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
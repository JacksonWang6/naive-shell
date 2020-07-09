#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../debug.h"

typedef enum {
    CP_FILE,
    CP_DIR,
    EMPTY,
} cp_file_t;

#define BUF_SIZE 1024
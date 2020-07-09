#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../debug.h"
/* 修改时间戳或者新建一个不存在的文件 */

void create_file(char* file_name);
void update_file(char* file_name);

struct timespec now_time[2];
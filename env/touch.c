#include "touch.h"

int main(int argc, char* argv[]) {
    /* check exist or not */
    if (argc == 1) {
        printf("touch: File operand is missing\n");
    }
    int i;/* the first one is cmd name, so skip it */
    for (i = 1; i < argc; i++) {
        int ret = access(argv[i], F_OK);
        if (ret == -1) {
            /* does not exist, create it */
            create_file(argv[i]);
        } else {
            /* exist, update timestap */
            update_file(argv[i]);
        }
    }
}

void create_file(char* file_name) {
    int fd = open(file_name, O_CREAT | O_RDWR, 0777);
    if (fd < 0) {
        printf("create file: %s failed.\n", file_name);
        return;
    }
    close(fd);
    return;
}

void update_file(char* file_name) {
    now_time[0].tv_nsec = UTIME_NOW;
    now_time[1].tv_nsec = UTIME_NOW;
    int ret = utimensat(AT_FDCWD, file_name, now_time, 0);
    if (ret == -1) {
        printf("update %s' timestap failed.\n", file_name);
    }
    return;
}
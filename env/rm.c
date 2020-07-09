#include "rm.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("rm: File operand is missing\n");
    }

    for (int i = 1; i < argc; i++) {
        char* cur = argv[i];
        int r = access(cur, F_OK);
        if (r == -1) {
            printf("can't delete %s: no that file or directory.\n", cur);
            continue;
        }

        struct stat file_stat;
        int ret = stat(cur, &file_stat);
        if (ret == -1) {
            printf("abnormal\n");
            return -1;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            printf("can't delete %s, it is a directory.\n", cur);
            continue;
        }

        ret = unlink(cur);
        if (ret == -1) {
            printf("rm %s failed.\n", cur);
        }
    }
}
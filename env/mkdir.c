#include "mkdir.h"

/* 不支持任何参数, 这是feature */

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("mkdir: Missing operand\n");
        return -1;
    }

    /* zsh中如果不加参数也是不支持 a/b这种创建的(当没有目录a的时候) */
    for (int i = 1; i < argc; i++) {
        /* first judge exist or not */
        int ret = access(argv[i], F_OK);
        if (ret != -1) {
            printf("mkdir: Can't create directory %s, File already exists\n", argv[i]);
            continue;
        }
        char* pos = strchr(argv[i], '/');
        if (pos != NULL) {
            printf("mkdir: can't create diretory: %s, No that file or directory\n", argv[i]);
            continue;
        } else {
            int ret = mkdir(argv[i], 0700);
            if (ret == -1) {
                printf("create directory %s failed.\n", argv[i]);
            }
        }
    }
}
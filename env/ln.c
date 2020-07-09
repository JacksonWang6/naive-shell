#include "ln.h"

int main(int argc, char* argv[]) {

    if (argc == 3) {
        /* hand link */
        char* s1 = argv[1];
        char* s2 = argv[2];
        /* must be file */
        int r1 = access(s1, F_OK);
        if (r1 == -1) {
            printf("failed to access %s, no such file or directory.\n", s1);
            return -1;
        }

        struct stat file_stat;
        int ret = stat(s1, &file_stat);
        if (ret == -1) {
            printf("abnormal\n");
            return -1;
        }

        if (S_ISDIR(file_stat.st_mode)) {
            printf("ln: %s, Do not allow hard links to the directory", s1);
            return -1;
        }

        int r2 = access(s2, F_OK);
        if (r2 == 0) {
            printf("ln: unable to create hard link %s, file already exists", s2);
            return -1;
        }
        ret = link(s1, s2);
        if (ret < 0) {
            printf("creat hand link failed.\n");
            return -1;
        }
    } else if (argc == 4) {
        /* symbol link */
        if (strcmp(argv[1], "-s") == 0 && strlen(argv[1]) == strlen("-s")) {
            char* s1 = argv[2];
            char* s2 = argv[3];
            int ret = access(s2, F_OK);
            if (ret == 0) {
                printf("ln: unable to create symbol link %s, file already exists", s2);
                return -1;
            }
            ret = symlink(s1, s2);
            if (ret < 0) {
                printf("create symbol link failed.\n");
                return -1;
            }
        } else {
            printf("argument error.\n");
            return -1;
        }
    } else {
        printf("argument error.\nplease input correct argument.\n");
        return -1;
    }
    return 0;
}
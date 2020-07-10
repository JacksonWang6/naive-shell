#include "cp.h"

/* 同样的,不支持任何参数,没有考虑带路径の文件名 */

int main(int argc, char* argv[]) {
    if (argc == 1) {
        printf("cp: File operand is missing\n");
        return -1;
    }

    if (argc == 2) {
        printf("cp: The target file to be operated is missing after %s\n", argv[1]);
        return -1;
    }

    if (argc == 3) {
        /* 分情况 */
        char* source = argv[1];
        char* target = argv[2];
        /* exist or not? */
        int r1 = access(source, F_OK);
        if (r1 == -1) {
            printf("cp: Unable to get the file status (stat) of %s : there is no file or directory\n", source);
            return -1;
        }
        int r2 = access(target, F_OK);
        /* judge type: file or dir */
        cp_file_t source_t, target_t;
        struct stat file_stat;
        int ret = stat(source, &file_stat);
        if (ret == -1) {
            printf("abnormal\n");
            return -1;
        }
        /* 这里其实可以用三目运算符骚一下 */
        if (S_ISDIR(file_stat.st_mode)) {
            source_t = CP_DIR;
        } else if (S_ISREG(file_stat.st_mode)) {
            source_t = CP_FILE;
        } else {
            printf("unsupported type\n");
            return -1;
        }
        /* check souce's type */
        if (source_t == CP_DIR) {
            printf("cp: -r not specified; omitting directory %s\n", source);
            return -1;
        } 

        if (r2 == -1) {
            target_t = EMPTY;
        } else {
            ret = stat(target, &file_stat);
            if (ret == -1) {
                printf("abnormal\n");
                return -1;
            }
            if (S_ISDIR(file_stat.st_mode)) {
                target_t = CP_DIR;
            } else if (S_ISREG(file_stat.st_mode)) {
                target_t = CP_FILE;
            } else {
                printf("unsupported type\n");
                return -1;
            }
        }

        /* cp source target */
        char buf[BUF_SIZE] = {0};
        int cnt = 0;
        if (target_t == CP_FILE || target_t == EMPTY) {
            /* cp file file */
            int source_fd = open(source, O_RDONLY);
            if (source_fd < 0) {
                printf("open %s failed.\n", source);
                return -1;
            }
            int target_fd = open(target, O_RDWR | O_CREAT, 0777);
            if (target_fd < 0) {
                printf("open %s failed.\n", target);
                return -1;
            }
            while(1) {
                cnt = read(source_fd, buf, BUF_SIZE);
                if(cnt < BUF_SIZE) break;
                write(target_fd, buf, cnt);
                memset(buf, 0, BUF_SIZE);
            }
            write(target_fd, buf, cnt);   /* 做最后一次写入 */
        } else {
            /* cp file dir */
            char target_file[BUF_SIZE] = {0};
            sprintf(target_file, "%s/%s", target, source);
            Log("target_file: %s", target_file);
            int source_fd = open(source, O_RDONLY);
            if (source_fd < 0) {
                printf("open %s failed.\n", source);
                return -1;
            }
            int target_fd = open(target_file, O_RDWR | O_CREAT, 0777);
            if (target_fd < 0) {
                printf("open %s failed.\n", target_file);
                return -1;
            }
            while(1) {
                cnt = read(source_fd, buf, BUF_SIZE);
                if(cnt < BUF_SIZE) break;
                write(target_fd, buf, cnt);
                memset(buf, 0, BUF_SIZE);
            }
            write(target_fd, buf, cnt);   /* 做最后一次写入 */
        }
    } else if (argc > 3) {
        /* check argv[argc-1] */
        char* target = argv[argc-1];
        int ret = access(target, F_OK);
        if (ret == -1) {
            printf("cp: target %s is not a directory\n", target);
            return -1;
        }

        struct stat file_stat;
        cp_file_t target_t, source_t;
        int r = stat(target, &file_stat);
        if (r == -1) {
            printf("abnormal\n");
            return -1;
        }
        if (S_ISDIR(file_stat.st_mode)) {
            target_t = CP_DIR;
        } else if (S_ISREG(file_stat.st_mode)) {
            target_t = CP_FILE;
        } else {
            printf("unsupported type\n");
            return -1;
        }

        if (target_t != CP_DIR) {
            printf("cp: target %s is not a directory\n", target);
            return -1;
        }

        /* traverse */
        char* source = NULL;
        for (int i = 1; i < argc - 1; i++) {
            source = argv[i];
            int ret = access(source, F_OK);
            if (ret == -1) {
                printf("cp: Unable to get the file status (stat) of %s : there is no file or directory\n", source);
                continue;
            }

            int r = stat(source, &file_stat);
            if (r == -1) {
                printf("abnormal\n");
                continue;
            }
            if (S_ISDIR(file_stat.st_mode)) {
                source_t = CP_DIR;
            } else if (S_ISREG(file_stat.st_mode)) {
                source_t = CP_FILE;
            } else {
                printf("unsupported type\n");
                continue;
            }

            if (source_t == CP_DIR) {
                printf("cp: -r not specified; omitting directory %s\n", source);
                continue;
            }
            /* write */
            int cnt = 0;
            char buf[BUF_SIZE] = {0};
            char target_file[BUF_SIZE] = {0};
            sprintf(target_file, "%s/%s", target, source);
            Log("target_file: %s", target_file);
            int source_fd = open(source, O_RDONLY);
            if (source_fd < 0) {
                printf("open %s failed.\n", source);
                continue;
            }
            int target_fd = open(target_file, O_RDWR | O_CREAT, 0777);
            if (target_fd < 0) {
                printf("open %s failed.\n", target_file);
                continue;
            }
            while(1) {
                cnt = read(source_fd, buf, BUF_SIZE);
                if(cnt < BUF_SIZE) break;
                write(target_fd, buf, cnt);
                memset(buf, 0, BUF_SIZE);
            }
            write(target_fd, buf, cnt);   /* 做最后一次写入 */
        }
    }
}
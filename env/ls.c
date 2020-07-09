#include "ls.h"

int main(int argc, char* argv[]) {
    success_g = true;
    /* test argc & argv */
    Log("%d", argc);
    for (int i = 0; i < argc; i++) {
        Log("argv[%d]: %s", i, argv[i]);
    }

    /* parse arguments */
    char* path = parse_arguments(argc, argv);
    if (!success_g) return 0;
    /* obtain pwd */
    char cur_path[BUF_SIZE] = {0};
    getcwd(cur_path, BUF_SIZE);
    Log("cur_path: %s", cur_path);
    if (path != NULL) {
        /* parse path */
        Log("path: %s", path);
        /* if start with '/', it is a absolute path, else a releative path */
        if (path[0] == '/') {
            strcpy(cur_path, path);
        } else {
            /* concat path */
            sprintf(cur_path, "%s/%s", cur_path, path);
        }
        Log("after concat: %s", cur_path);
    }

    /* open dir and parse files */
    parse_dir(cur_path);
}

/* return value: if argu contions path, then return path, otherwise return null */
char* parse_arguments(int argc, char* argv[]) {
    bool ispath = false;
    char* path = NULL;
    for (int i = 1; i < argc; i++) {
        Assert(argv[i] != NULL, "error");
        /* if start with '-', it maybe argument, else it maybe path */
        if (argv[i][0] == '-') {
            int j;
            for (j = 0; j < NR_OPTIONS; j++) {
                if (strcmp(argv[i], options[j].name) == 0 || strcmp(argv[i], options[j].full_name) == 0) {
                    *(options[j].target) = true;
                    break;
                }
            }
            if (j == NR_OPTIONS) {
                printf("Unknown argument: %s\n", argv[i]);
                success_g = false;
                return NULL;
            }
        } else {
            if (ispath == false) {
                ispath = true;
                path = (char*) malloc (sizeof(char) * strlen(argv[i]) * 2);
                path = argv[i];
            } else {
                printf("[error] Usage: ls [options] [path]\npath only 0 or 1\n");
                success_g = false;
                return NULL;
            }
        }
    }
    return path;
}

void parse_dir(char* cur_path) {
    DIR* dir = opendir(cur_path);
    if (dir == NULL) {
        success_g = false;
        printf("open %s failed!\n", cur_path);
        return;
    }

    struct dirent* ptr;
    struct stat file_stat;
    file_info_t info;
    while ((ptr = readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;
        strcpy(info.name, ptr->d_name);
        Log("%s", info.name);
        info.l = false;
        char result_str[256] = {0};
        if (OP_LIST) {
            info.l = true;
            Log("run here");
            strcpy(info.mode, "----------");
            int ret = stat(info.name, &file_stat);
            /* error handler */
            if (ret == -1) {
                success_g = false;
                printf("parse_dir error\n");
                return;
            }
            Log("run here");
            /* get detailed info */
            /* https://blog.csdn.net/wooden954/article/details/2442435 */
            int b_mask = file_stat.st_mode&S_IFMT;
            if(b_mask==S_IFDIR) {
                info.mode[0]='d';
            }
            else if(b_mask==S_IFREG){
                info.mode[0]='-';
            }

            info.mode[1]=(file_stat.st_mode&S_IRUSR)?'r':'-';
            info.mode[2]=(file_stat.st_mode&S_IWUSR)?'w':'-';
            info.mode[3]=(file_stat.st_mode&S_IXUSR)?'x':'-';
            info.mode[4]=(file_stat.st_mode&S_IRGRP)?'r':'-';
            info.mode[5]=(file_stat.st_mode&S_IWGRP)?'w':'-';
            info.mode[6]=(file_stat.st_mode&S_IXGRP)?'x':'-';
            info.mode[7]=(file_stat.st_mode&S_IROTH)?'r':'-';
            info.mode[8]=(file_stat.st_mode&S_IWOTH)?'w':'-';
            info.mode[9]=(file_stat.st_mode&S_IXOTH)?'x':'-';

            info.gid = file_stat.st_gid;
            info.uid = file_stat.st_uid;
            Log("run here");
            info.size = file_stat.st_size;
            strftime(info.date,13,"%b %d %H:%M",localtime(&(file_stat.st_mtime)));
            Log("run here");
            Log("gid: %d, uid: %d, size: %ld, name: %s", info.gid, \
                                                            info.uid, info.size, info.name);
            // TODO();
            sprintf(result_str,"%s %5d %5d %6ld %12s %s\n",info.mode, info.uid, info.gid, info.size, info.date, info.name);
        } else {
            sprintf(result_str, "%s\t", info.name);
        }

        printf("%s",result_str);
    }
}
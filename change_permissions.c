#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define TARGET_UID 8675309
#define TARGET_GID 8675309
// 770
#define TARGET_MODE (S_IRWXU|S_IRWXG)
// We can't have paths longer than entry->d_name
#define MAX_PATH_LEN MAXNAMLEN + 1

void change_permission_if_needed(const char *path) {
        struct stat sb;
        if (lstat(path, &sb) == -1) {
                fprintf(stderr, "ERR: stat failed on file: %s\n", path);
                return;
        }
        if (!S_ISREG(sb.st_mode) && !S_ISDIR(sb.st_mode)) {
                fprintf(
                        stderr,
                        "WARN: %s is not a file or directory, skipping\n",
                        path);
                return;
        }

        // Maybe change owner/group
        if (sb.st_uid != TARGET_UID || sb.st_gid != TARGET_GID) {
                chown(path, TARGET_UID, TARGET_GID);
                printf("Changed owner/group for %s\n", path);
        }

        // Maybe change mode
        if ((sb.st_mode & TARGET_MODE) != TARGET_MODE) {
                chmod(path, TARGET_MODE);
                printf("Changed mode for %s\n", path);
        }
}

int add_trailing_slash(char *path) {
        // Assume we allocated a string with fixed buffer size
        int len = strnlen(path, MAX_PATH_LEN);
        if (len >= MAX_PATH_LEN) {
                return 0;
        }
        if (path[len - 1] != '/') {
                path[len] = '/';
                path[len + 1] = '\0';
        }
        return 1;
}

void change_permissions(char *path) {
        if (!add_trailing_slash(path)) {
                fprintf(stderr, "ERR: dir too large (stopping)\n");
                return;
        }
        DIR *dir = opendir(path);
        if (!dir) {
                fprintf(stderr, "ERR: opendir(\"%s\") failed.\n", path);
                return;
        }
        struct dirent *entry;
        while ((entry = readdir(dir))) {
                // Ignore hidden files
                if (entry->d_name[0] == '.') {
                        continue;
                }
                char full_path[MAX_PATH_LEN];
                snprintf(full_path, MAX_PATH_LEN, "%s%s", path, entry->d_name);
                change_permission_if_needed(full_path);
                // Recurse on directory
                if (entry->d_type == DT_DIR) {
                        change_permissions(full_path);
                }
        }
        closedir(dir);
}

int main(int argc, char **argv) {
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
                return 1;
        }
        char path[MAX_PATH_LEN];
        snprintf(path, MAX_PATH_LEN, "%s", argv[1]);
        change_permissions(path);
        return 0;
}

#include "../include/display.h"

void print_prompt(const char *username, const char *sysname, const char *home_dir, const char *cwd) {
    if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
        // Inside home directory
        const char *rel = cwd + strlen(home_dir);
        if (*rel == '\0') {
            printf("<%s@%s:~> ", username, sysname);
        } else if (*rel == '/') {
            rel++; // skip the slash
            printf("<%s@%s:~/%s> ", username, sysname, rel);
        } else {
            printf("<%s@%s:~%s> ", username, sysname, rel);
        }
    } else {
        // Outside home directory
        printf("<%s@%s:%s> ", username, sysname, cwd);
    }
    fflush(stdout);
}

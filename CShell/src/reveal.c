#include "../include/reveal.h"
#include <time.h>
#include <grp.h>

int compare_entries(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void print_permissions(mode_t mode) {
    printf((S_ISDIR(mode)) ? "d" : "-");
    printf((mode & S_IRUSR) ? "r" : "-");
    printf((mode & S_IWUSR) ? "w" : "-");
    printf((mode & S_IXUSR) ? "x" : "-");
    printf((mode & S_IRGRP) ? "r" : "-");
    printf((mode & S_IWGRP) ? "w" : "-");
    printf((mode & S_IXGRP) ? "x" : "-");
    printf((mode & S_IROTH) ? "r" : "-");
    printf((mode & S_IWOTH) ? "w" : "-");
    printf((mode & S_IXOTH) ? "x" : "-");
}

void print_file_info(const char *path, const char *name, bool show_hidden, bool long_format) {
    if (!show_hidden && name[0] == '.') {
        return;
    }
    
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, name);
    
    struct stat file_stat;
    if (stat(full_path, &file_stat) == -1) {
        return;
    }
    
    if (long_format) {
        // Print permissions
        print_permissions(file_stat.st_mode);
        
        // Print number of links
        printf(" %2ld", file_stat.st_nlink);
        
        // Print owner and group
        struct passwd *pw = getpwuid(file_stat.st_uid);
        struct group *gr = getgrgid(file_stat.st_gid);
        printf(" %s %s", pw ? pw->pw_name : "unknown", gr ? gr->gr_name : "unknown");
        
        // Print size
        printf(" %8ld", file_stat.st_size);
        
        // Print modification time
        char time_str[64];
        struct tm *tm_info = localtime(&file_stat.st_mtime);
        strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);
        printf(" %s", time_str);
        
        // Print name with color coding
        if (S_ISDIR(file_stat.st_mode)) {
            printf(" \033[1;34m%s\033[0m\n", name);  // Blue for directories
        } else if (file_stat.st_mode & S_IXUSR) {
            printf(" \033[1;32m%s\033[0m\n", name);  // Green for executables
        } else {
            printf(" %s\n", name);
        }
    } else {
        // Simple format with color coding
        if (S_ISDIR(file_stat.st_mode)) {
            printf("\033[1;34m%s\033[0m\n", name);  // Blue for directories
        } else if (file_stat.st_mode & S_IXUSR) {
            printf("\033[1;32m%s\033[0m\n", name);  // Green for executables
        } else {
            printf("%s\n", name);
        }
    }
}

bool handle_reveal_command(char **args, int arg_count) {
    if (arg_count == 0 || strcmp(args[0], "reveal") != 0) {
        return false;
    }
    
    bool show_hidden = false;
    bool long_format = false;
    char target_path[PATH_MAX] = ".";  // Default to current directory
    
    // Parse flags and path
    for (int i = 1; i < arg_count; i++) {
        if (args[i][0] == '-') {
            for (int j = 1; args[i][j] != '\0'; j++) {
                switch (args[i][j]) {
                    case 'a':
                        show_hidden = true;
                        break;
                    case 'l':
                        long_format = true;
                        break;
                    default:
                        printf("reveal: invalid option -- '%c'\n", args[i][j]);
                        return true;
                }
            }
        } else {
            // It's a path
            strcpy(target_path, args[i]);
        }
    }
    
    // Expand ~ to home directory
    if (target_path[0] == '~') {
        char expanded_path[PATH_MAX];
        if (target_path[1] == '\0') {
            strcpy(expanded_path, shell_home);
        } else if (target_path[1] == '/') {
            snprintf(expanded_path, sizeof(expanded_path), "%s%s", actual_home, target_path + 1);
        } else {
            strcpy(expanded_path, target_path);
        }
        strcpy(target_path, expanded_path);
    }
    
    struct stat path_stat;
    if (stat(target_path, &path_stat) == -1) {
        perror("reveal");
        return true;
    }
    
    if (S_ISDIR(path_stat.st_mode)) {
        // Directory listing
        DIR *dir = opendir(target_path);
        if (!dir) {
            perror("reveal");
            return true;
        }
        
        // Collect all entries
        char **entries = malloc(1000 * sizeof(char*));
        int entry_count = 0;
        
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (!show_hidden && entry->d_name[0] == '.') {
                continue;
            }
            entries[entry_count] = malloc(strlen(entry->d_name) + 1);
            strcpy(entries[entry_count], entry->d_name);
            entry_count++;
        }
        closedir(dir);
        
        // Sort entries
        qsort(entries, entry_count, sizeof(char*), compare_entries);
        
        if (long_format) {
            // Print total
            printf("total %d\n", entry_count);
        }
        
        // Print entries
        for (int i = 0; i < entry_count; i++) {
            print_file_info(target_path, entries[i], show_hidden, long_format);
            free(entries[i]);
        }
        free(entries);
    } else {
        // Single file
        char *dir_name = strdup(target_path);
        char *base_name = strdup(target_path);
        char *dir_part = dirname(dir_name);
        char *file_part = basename(base_name);
        
        print_file_info(dir_part, file_part, true, long_format);
        
        free(dir_name);
        free(base_name);
    }
    
    return true;
}

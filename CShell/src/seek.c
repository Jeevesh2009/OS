#include "../include/seek.h"

static char found_files[1000][1024];
static int found_count = 0;

bool matches_target(const char *path, const char *target) {
    char *filename = strrchr(path, '/');
    if (filename) {
        filename++; // Skip the '/'
    } else {
        filename = (char *)path;
    }
    
    return strstr(filename, target) != NULL;
}

void search_recursive(const char *dir_path, const char *target, 
                     bool dirs_only, bool files_only, bool execute_flag) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        return; // No permission or doesn't exist
    }
    
    struct dirent *entry;
    char full_path[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) == -1) {
            continue;
        }
        
        bool is_dir = S_ISDIR(file_stat.st_mode);
        bool is_file = S_ISREG(file_stat.st_mode);
        
        // Check if this item matches our search criteria
        if (matches_target(entry->d_name, target)) {
            bool should_print = false;
            
            if (dirs_only && is_dir) {
                should_print = true;
            } else if (files_only && is_file) {
                should_print = true;
            } else if (!dirs_only && !files_only) {
                should_print = true;
            }
            
            if (should_print) {
                // Print relative to current directory if possible
                char *relative_path = full_path;
                char current_dir[1024];
                getcwd(current_dir, sizeof(current_dir));
                
                if (strncmp(full_path, current_dir, strlen(current_dir)) == 0) {
                    relative_path = full_path + strlen(current_dir);
                    if (relative_path[0] == '/') relative_path++;
                    if (relative_path[0] == '\0') relative_path = ".";
                }
                
                if (is_dir) {
                    printf("\033[1;34m%s\033[0m\n", relative_path); // Blue for directories
                } else {
                    printf("\033[1;32m%s\033[0m\n", relative_path); // Green for files
                }
                
                // Store for execution check
                if (found_count < 1000) {
                    strcpy(found_files[found_count], full_path);
                    found_count++;
                }
            }
        }
        
        // Recursively search subdirectories
        if (is_dir) {
            search_recursive(full_path, target, dirs_only, files_only, execute_flag);
        }
    }
    
    closedir(dir);
}

void handle_seek_command(char **args, int arg_count) {
    bool dirs_only = false;
    bool files_only = false;
    bool execute_flag = false;
    char *target = NULL;
    char *search_dir = "."; // Default to current directory
    
    // Reset found files
    found_count = 0;
    
    // Parse arguments
    for (int i = 1; i < arg_count; i++) {
        if (strcmp(args[i], "-d") == 0) {
            dirs_only = true;
        } else if (strcmp(args[i], "-f") == 0) {
            files_only = true;
        } else if (strcmp(args[i], "-e") == 0) {
            execute_flag = true;
        } else if (target == NULL) {
            target = args[i];
        } else {
            search_dir = args[i];
        }
    }
    
    if (target == NULL) {
        printf("Usage: seek [-d] [-f] [-e] target [directory]\n");
        printf("  -d: Search only directories\n");
        printf("  -f: Search only files\n");
        printf("  -e: Execute/change to found item if exactly one match\n");
        return;
    }
    
    if (dirs_only && files_only) {
        printf("Invalid flags!\n");
        return;
    }
    
    // Convert relative path to absolute if needed
    char abs_search_dir[1024];
    if (search_dir[0] != '/') {
        char current_dir[1024];
        getcwd(current_dir, sizeof(current_dir));
        
        if (strcmp(search_dir, ".") == 0) {
            strcpy(abs_search_dir, current_dir);
        } else if (strcmp(search_dir, "~") == 0) {
            strcpy(abs_search_dir, actual_home);
        } else {
            snprintf(abs_search_dir, sizeof(abs_search_dir), "%s/%s", current_dir, search_dir);
        }
    } else {
        strcpy(abs_search_dir, search_dir);
    }
    
    printf("Searching for '%s' in %s\n", target, abs_search_dir);
    search_recursive(abs_search_dir, target, dirs_only, files_only, execute_flag);
    
    // Handle execution flag
    if (execute_flag && found_count == 1) {
        struct stat file_stat;
        if (stat(found_files[0], &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                if (chdir(found_files[0]) == 0) {
                    printf("Changed directory to: %s\n", found_files[0]);
                    
                    // Update prev_dir
                    char current_dir[1024];
                    getcwd(current_dir, sizeof(current_dir));
                    strcpy(prev_dir, current_dir);
                } else {
                    printf("Missing permissions for task!\n");
                }
            } else if (S_ISREG(file_stat.st_mode)) {
                // Check if file is executable
                if (file_stat.st_mode & S_IXUSR) {
                    printf("Executing: %s\n", found_files[0]);
                    system(found_files[0]);
                } else {
                    printf("Missing permissions for task!\n");
                }
            }
        }
    } else if (execute_flag && found_count == 0) {
        printf("No match found!\n");
    } else if (execute_flag && found_count > 1) {
        printf("Multiple matches found!\n");
    }
}

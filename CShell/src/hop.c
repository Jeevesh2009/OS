#include "../include/hop.h"
#include "../include/display.h"

bool handle_hop_command(char **args, int arg_count) {
    if (arg_count == 0 || strcmp(args[0], "hop") != 0) {
        return false;
    }
    
    char current_dir[PATH_MAX];
    if (!getcwd(current_dir, sizeof(current_dir))) {
        perror("getcwd");
        return true;
    }
    
    // If no arguments, hop to shell home directory
    if (arg_count == 1) {
        strcpy(prev_dir, current_dir);
        if (chdir(shell_home) == 0) {
            if (getcwd(current_dir, sizeof(current_dir))) {
                printf("%s\n", current_dir);
            }
        } else {
            perror("hop");
        }
        return true;
    }
    
    // Process each argument sequentially
    for (int i = 1; i < arg_count; i++) {
        char* target_dir = args[i];
        char resolved_path[PATH_MAX];
        
        // Update current directory before changing
        if (!getcwd(current_dir, sizeof(current_dir))) {
            perror("getcwd");
            continue;
        }
        
        // Handle special cases
        if (strcmp(target_dir, "~") == 0) {
            strcpy(resolved_path, shell_home);
        } else if (strcmp(target_dir, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                printf("hop: OLDPWD not set\n");
                continue;
            }
            strcpy(resolved_path, prev_dir);
        } else if (strcmp(target_dir, ".") == 0) {
            strcpy(resolved_path, current_dir);
        } else if (strcmp(target_dir, "..") == 0) {
            strcpy(resolved_path, "..");
        } else if (target_dir[0] == '~' && target_dir[1] == '/') {
            // Handle ~/path format - use actual home directory for expansion
            snprintf(resolved_path, sizeof(resolved_path), "%s%s", actual_home, target_dir + 1);
        } else {
            // Use the path as-is (relative or absolute)
            strcpy(resolved_path, target_dir);
        }
        
        // Store current directory as previous before changing
        strcpy(prev_dir, current_dir);
        
        // Change directory
        if (chdir(resolved_path) == 0) {
            // Get and print the new working directory
            char new_cwd[PATH_MAX];
            if (getcwd(new_cwd, sizeof(new_cwd))) {
                printf("%s\n", new_cwd);
            } else {
                perror("getcwd");
            }
        } else {
            perror("hop");
            // Restore previous directory on failure by not updating prev_dir
            strcpy(prev_dir, "");
        }
    }
    
    return true;
}

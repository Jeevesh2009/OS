#include "../include/log.h"

static char log_entries[MAX_LOG_ENTRIES][SHELL_MAX_INPUT];
static int log_count = 0;
static char log_file_path[PATH_MAX];

void initialize_log() {
    snprintf(log_file_path, sizeof(log_file_path), "%s/.cshell_log", actual_home);
    load_log();
}

void load_log() {
    FILE *file = fopen(log_file_path, "r");
    if (!file) {
        return;  // No log file exists yet
    }
    
    log_count = 0;
    char line[SHELL_MAX_INPUT];
    while (fgets(line, sizeof(line), file) && log_count < MAX_LOG_ENTRIES) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        strcpy(log_entries[log_count], line);
        log_count++;
    }
    fclose(file);
}

void save_log() {
    FILE *file = fopen(log_file_path, "w");
    if (!file) {
        perror("Failed to save log");
        return;
    }
    
    for (int i = 0; i < log_count; i++) {
        fprintf(file, "%s\n", log_entries[i]);
    }
    fclose(file);
}

void add_to_log(char **args, int arg_count) {
    if (arg_count == 0) return;
    
    // Reconstruct the command from args
    char command[SHELL_MAX_INPUT] = "";
    for (int i = 0; i < arg_count; i++) {
        if (i > 0) strcat(command, " ");
        strcat(command, args[i]);
    }
    
    // Don't log empty commands or log command itself
    if (strlen(command) == 0 || strncmp(command, "log", 3) == 0) {
        return;
    }
    
    // Check if command is the same as the last entry
    if (log_count > 0 && strcmp(log_entries[log_count - 1], command) == 0) {
        return;
    }
    
    // If log is full, shift entries
    if (log_count >= MAX_LOG_ENTRIES) {
        for (int i = 0; i < MAX_LOG_ENTRIES - 1; i++) {
            strcpy(log_entries[i], log_entries[i + 1]);
        }
        log_count = MAX_LOG_ENTRIES - 1;
    }
    
    // Add new entry
    strcpy(log_entries[log_count], command);
    log_count++;
    
    save_log();
}

void print_log() {
    for (int i = 0; i < log_count; i++) {
        printf("%d. %s\n", i + 1, log_entries[i]);
    }
}

void execute_log_entry(int index) {
    if (index < 1 || index > log_count) {
        printf("log: invalid index %d\n", index);
        return;
    }
    
    printf("Executing: %s\n", log_entries[index - 1]);
    // TODO: Parse and execute the command
    // This would require access to the main command parsing function
}

void purge_log() {
    log_count = 0;
    save_log();
    printf("Log purged.\n");
}

void handle_log_command(char **args, int arg_count) {
    if (arg_count == 1) {
        // Just "log" - print all entries
        print_log();
    } else if (arg_count == 2) {
        if (strcmp(args[1], "purge") == 0) {
            purge_log();
        } else {
            // Try to parse as index for execution
            int index = atoi(args[1]);
            if (index > 0) {
                execute_log_entry(index);
            } else {
                printf("log: invalid argument '%s'\n", args[1]);
            }
        }
    } else {
        printf("log: too many arguments\n");
    }
}

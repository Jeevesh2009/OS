#include "../include/proclore.h"

void print_process_info(pid_t pid) {
    char stat_path[256], exe_path[256], status_path[256];
    char exe_link[1024];
    FILE *stat_file, *status_file;
    
    // Construct paths
    snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
    snprintf(exe_path, sizeof(exe_path), "/proc/%d/exe", pid);
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", pid);
    
    // Read stat file for basic info
    stat_file = fopen(stat_path, "r");
    if (!stat_file) {
        printf("No such process found\n");
        return;
    }
    
    // Parse stat file
    int stat_pid;
    char comm[256], state;
    int ppid;
    unsigned long vmsize;
    
    fscanf(stat_file, "%d %s %c %d", &stat_pid, comm, &state, &ppid);
    
    // Skip to vmsize (field 23)
    for (int i = 4; i < 22; i++) {
        char temp[256];
        fscanf(stat_file, "%s", temp);
    }
    fscanf(stat_file, "%lu", &vmsize);
    fclose(stat_file);
    
    // Read executable path
    ssize_t len = readlink(exe_path, exe_link, sizeof(exe_link) - 1);
    if (len == -1) {
        strcpy(exe_link, "N/A");
    } else {
        exe_link[len] = '\0';
    }
    
    // Check if process group is same as terminal (foreground check)
    char *process_status = (state == 'R' || state == 'S') ? "R+" : "R";
    
    printf("pid : %d\n", pid);
    printf("process Status : %c%s\n", state, 
           (getpgid(pid) == tcgetpgrp(STDIN_FILENO)) ? "+" : "");
    printf("memory : %lu\n", vmsize);
    printf("Executable Path : %s\n", exe_link);
}

void handle_proclore_command(char **args, int arg_count) {
    pid_t pid;
    
    if (arg_count == 1) {
        // No PID provided, use shell's PID
        pid = getpid();
    } else if (arg_count == 2) {
        // PID provided
        pid = atoi(args[1]);
        if (pid <= 0) {
            printf("Invalid PID\n");
            return;
        }
    } else {
        printf("Usage: proclore [pid]\n");
        return;
    }
    
    print_process_info(pid);
}

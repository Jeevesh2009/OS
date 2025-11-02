#include "../include/neonate.h"
#include <time.h>

void handle_neonate_command(char **args, int arg_count) {
    if (arg_count < 3 || strcmp(args[1], "-n") != 0) {
        printf("Usage: neonate -n [time_arg]\n");
        return;
    }
    
    int time_interval = atoi(args[2]);
    if (time_interval <= 0) {
        printf("Invalid time interval\n");
        return;
    }
    
    printf("Press 'x' to exit\n");
    
    // Set terminal to non-canonical mode
    struct termios old_termios, new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    new_termios.c_cc[VTIME] = 0;
    new_termios.c_cc[VMIN] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    time_t last_print = 0;
    char ch;
    
    while (1) {
        time_t current_time = time(NULL);
        
        // Print PID if enough time has passed
        if (current_time - last_print >= time_interval) {
            print_most_recent_pid();
            last_print = current_time;
        }
        
        // Check for 'x' key press
        if (read(STDIN_FILENO, &ch, 1) > 0 && ch == 'x') {
            break;
        }
        
        usleep(100000); // Sleep for 100ms to avoid busy waiting
    }
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    printf("\nNeonate terminated\n");
}

void print_most_recent_pid() {
    int pid = get_most_recent_pid();
    if (pid > 0) {
        printf("%d\n", pid);
    } else {
        printf("No processes found\n");
    }
}

int get_most_recent_pid() {
    DIR *proc_dir = opendir("/proc");
    if (!proc_dir) {
        return -1;
    }
    
    int max_pid = 0;
    struct dirent *entry;
    
    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if directory name is a number (PID)
        char *endptr;
        int pid = strtol(entry->d_name, &endptr, 10);
        
        if (*endptr == '\0' && pid > max_pid) {
            // Check if process still exists by trying to access its stat file
            char stat_path[256];
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
            
            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file) {
                max_pid = pid;
                fclose(stat_file);
            }
        }
    }
    
    closedir(proc_dir);
    return max_pid;
}

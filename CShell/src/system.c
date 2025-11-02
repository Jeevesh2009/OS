#include "../include/system.h"
#include "../include/activities.h"
#include "../include/signals.h"

typedef struct {
    pid_t pid;
    char command[256];
} BackgroundProcess;

static BackgroundProcess bg_processes[100];
static int bg_count = 0;

void add_background_process(pid_t pid, const char *command) {
    if (bg_count < 100) {
        bg_processes[bg_count].pid = pid;
        strncpy(bg_processes[bg_count].command, command, 255);
        bg_processes[bg_count].command[255] = '\0';
        bg_count++;
    }
}

void handle_background_processes() {
    for (int i = 0; i < bg_count; i++) {
        int status;
        pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG);
        
        if (result > 0) {
            // Process finished
            if (WIFEXITED(status)) {
                printf("%s with pid %d exited normally\n", 
                       bg_processes[i].command, bg_processes[i].pid);
            } else if (WIFSIGNALED(status)) {
                printf("%s with pid %d exited abnormally\n", 
                       bg_processes[i].command, bg_processes[i].pid);
            }
            
            // Remove from list
            for (int j = i; j < bg_count - 1; j++) {
                bg_processes[j] = bg_processes[j + 1];
            }
            bg_count--;
            i--; // Adjust index after removal
        }
    }
}

void execute_system_command(char **args, int arg_count, bool background) {
    if (arg_count == 0) return;
    
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        if (background) {
            // Set process group for background processes
            setpgid(0, 0);
        }
        
        if (execvp(args[0], args) == -1) {
            printf("ERROR : '%s' is not a valid command\n", args[0]);
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        if (background) {
            printf("[%d] %d\n", bg_count + 1, pid);
            add_background_process(pid, args[0]);
            add_activity(pid, args[0]);
        } else {
            foreground_pid = pid;
            foreground_job_id = 1; // Simplified job numbering
            int status;
            waitpid(pid, &status, WUNTRACED);
            
            if (WIFSTOPPED(status)) {
                // Process was stopped (Ctrl+Z)
                add_activity(pid, args[0]);
                set_activity_state(pid, "Stopped");
            }
            
            foreground_pid = 0;
            foreground_job_id = 0;
        }
    } else {
        perror("fork");
    }
}

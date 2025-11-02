#include "../include/signals.h"
#include "../include/activities.h"

pid_t foreground_pid = 0;
int foreground_job_id = 0;

void setup_signal_handlers() {
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    signal(SIGCHLD, handle_sigchld);
}

void handle_sigint(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    if (foreground_pid > 0) {
        // Send SIGINT to foreground process
        kill(foreground_pid, SIGINT);
        foreground_pid = 0;
        foreground_job_id = 0;
    }
    printf("\n");
}

void handle_sigtstp(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    if (foreground_pid > 0) {
        // Send SIGTSTP to foreground process
        kill(foreground_pid, SIGTSTP);
        
        // Add to activities as stopped
        char command[256];
        snprintf(command, sizeof(command), "PID %d", foreground_pid);
        add_activity(foreground_pid, command);
        set_activity_state(foreground_pid, "Stopped");
        
        printf("\n[%d]+ Stopped\t\t%s\n", foreground_job_id, command);
        
        foreground_pid = 0;
        foreground_job_id = 0;
    }
    printf("\n");
}

void handle_sigchld(int sig) {
    (void)sig; // Suppress unused parameter warning
    
    int status;
    pid_t pid;
    
    // Reap all available zombie children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        ProcessActivity *activity = find_activity_by_pid(pid);
        if (activity) {
            if (WIFEXITED(status)) {
                printf("\n%s with pid %d exited normally\n", activity->command, pid);
            } else if (WIFSIGNALED(status)) {
                printf("\n%s with pid %d exited abnormally\n", activity->command, pid);
            }
            remove_activity(pid);
        }
    }
}

void send_signal_to_process(pid_t pid, int signal) {
    if (kill(pid, signal) == -1) {
        perror("kill");
    } else {
        printf("Sent signal %d to process %d\n", signal, pid);
    }
}

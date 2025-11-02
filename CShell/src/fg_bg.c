#include "../include/fg_bg.h"
#include "../include/activities.h"
#include "../include/signals.h"

void handle_fg_command(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Usage: fg <pid>\n");
        return;
    }
    
    pid_t pid = atoi(args[1]);
    if (pid <= 0) {
        printf("Invalid PID\n");
        return;
    }
    
    ProcessActivity *activity = find_activity_by_pid(pid);
    if (!activity) {
        printf("No such job found\n");
        return;
    }
    
    bring_to_foreground(pid);
}

void handle_bg_command(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Usage: bg <pid>\n");
        return;
    }
    
    pid_t pid = atoi(args[1]);
    if (pid <= 0) {
        printf("Invalid PID\n");
        return;
    }
    
    ProcessActivity *activity = find_activity_by_pid(pid);
    if (!activity) {
        printf("No such job found\n");
        return;
    }
    
    send_to_background(pid);
}

void bring_to_foreground(pid_t pid) {
    ProcessActivity *activity = find_activity_by_pid(pid);
    if (!activity) {
        printf("No such job found\n");
        return;
    }
    
    // Send SIGCONT to resume the process if it's stopped
    if (strcmp(activity->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
    }
    
    // Set as foreground process
    foreground_pid = pid;
    foreground_job_id = activity->job_id;
    
    // Give terminal control to the process
    tcsetpgrp(STDIN_FILENO, getpgid(pid));
    
    printf("Brought job %d (pid %d) to foreground\n", activity->job_id, pid);
    
    // Wait for the process to complete
    int status;
    waitpid(pid, &status, WUNTRACED);
    
    // Take back terminal control
    tcsetpgrp(STDIN_FILENO, getpgrp());
    
    // Update process state
    if (WIFEXITED(status) || WIFSIGNALED(status)) {
        remove_activity(pid);
    } else if (WIFSTOPPED(status)) {
        set_activity_state(pid, "Stopped");
    }
    
    foreground_pid = 0;
    foreground_job_id = 0;
}

void send_to_background(pid_t pid) {
    ProcessActivity *activity = find_activity_by_pid(pid);
    if (!activity) {
        printf("No such job found\n");
        return;
    }
    
    // Send SIGCONT to resume the process if it's stopped
    if (strcmp(activity->state, "Stopped") == 0) {
        kill(pid, SIGCONT);
        set_activity_state(pid, "Running");
        printf("Resumed job %d (pid %d) in background\n", activity->job_id, pid);
    } else {
        printf("Job %d (pid %d) is already running in background\n", activity->job_id, pid);
    }
}

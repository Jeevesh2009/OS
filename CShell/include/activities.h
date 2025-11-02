#ifndef ACTIVITIES_H
#define ACTIVITIES_H

#include "cshell.h"

// Structure for process activity
typedef struct {
    pid_t pid;
    char command[256];
    char state[10];  // Running/Stopped
    int job_id;
} ProcessActivity;

// Function declarations
void handle_activities_command(char **args, int arg_count);
void update_activities();
void add_activity(pid_t pid, const char *command);
void remove_activity(pid_t pid);
void set_activity_state(pid_t pid, const char *state);
ProcessActivity* find_activity_by_pid(pid_t pid);
ProcessActivity* find_activity_by_job_id(int job_id);

#endif

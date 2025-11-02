#include "../include/activities.h"

static ProcessActivity activities[100];
static int activity_count = 0;
static int next_job_id = 1;

void add_activity(pid_t pid, const char *command) {
    if (activity_count < 100) {
        activities[activity_count].pid = pid;
        strncpy(activities[activity_count].command, command, 255);
        activities[activity_count].command[255] = '\0';
        strcpy(activities[activity_count].state, "Running");
        activities[activity_count].job_id = next_job_id++;
        activity_count++;
    }
}

void remove_activity(pid_t pid) {
    for (int i = 0; i < activity_count; i++) {
        if (activities[i].pid == pid) {
            // Shift remaining activities
            for (int j = i; j < activity_count - 1; j++) {
                activities[j] = activities[j + 1];
            }
            activity_count--;
            break;
        }
    }
}

void set_activity_state(pid_t pid, const char *state) {
    for (int i = 0; i < activity_count; i++) {
        if (activities[i].pid == pid) {
            strncpy(activities[i].state, state, 9);
            activities[i].state[9] = '\0';
            break;
        }
    }
}

ProcessActivity* find_activity_by_pid(pid_t pid) {
    for (int i = 0; i < activity_count; i++) {
        if (activities[i].pid == pid) {
            return &activities[i];
        }
    }
    return NULL;
}

ProcessActivity* find_activity_by_job_id(int job_id) {
    for (int i = 0; i < activity_count; i++) {
        if (activities[i].job_id == job_id) {
            return &activities[i];
        }
    }
    return NULL;
}

void update_activities() {
    for (int i = 0; i < activity_count; i++) {
        int status;
        pid_t result = waitpid(activities[i].pid, &status, WNOHANG);
        
        if (result > 0) {
            // Process finished
            remove_activity(activities[i].pid);
            i--; // Adjust index after removal
        } else if (result == 0) {
            // Process still running - check if it's stopped
            char stat_path[256];
            snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", activities[i].pid);
            
            FILE *stat_file = fopen(stat_path, "r");
            if (stat_file) {
                char state;
                int pid;
                char comm[256];
                fscanf(stat_file, "%d %s %c", &pid, comm, &state);
                fclose(stat_file);
                
                if (state == 'T') {
                    strcpy(activities[i].state, "Stopped");
                } else {
                    strcpy(activities[i].state, "Running");
                }
            }
        }
    }
}

int compare_activities(const void *a, const void *b) {
    const ProcessActivity *act_a = (const ProcessActivity *)a;
    const ProcessActivity *act_b = (const ProcessActivity *)b;
    return strcmp(act_a->command, act_b->command);
}

void handle_activities_command(char **args, int arg_count) {
    if (arg_count != 1) {
        printf("Usage: activities\n");
        return;
    }
    
    update_activities();
    
    // Sort activities by command name
    qsort(activities, activity_count, sizeof(ProcessActivity), compare_activities);
    
    for (int i = 0; i < activity_count; i++) {
        printf("%d : %s - %s\n", 
               activities[i].pid, 
               activities[i].command, 
               activities[i].state);
    }
}

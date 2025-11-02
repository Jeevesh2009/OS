#ifndef SYSTEM_H
#define SYSTEM_H

#include "cshell.h"

// Function to execute system commands (non-builtin)
void execute_system_command(char **args, int arg_count, bool background);

// Helper functions for process management
void handle_background_processes();
void add_background_process(pid_t pid, const char *command);

#endif

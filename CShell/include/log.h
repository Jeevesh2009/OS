#ifndef LOG_H
#define LOG_H

#include "cshell.h"

#define MAX_LOG_ENTRIES 15

// Function to handle log command
void handle_log_command(char **args, int arg_count);

// Helper functions
void initialize_log();
void add_to_log(char **args, int arg_count);
void load_log();
void save_log();
void print_log();
void execute_log_entry(int index);

#endif

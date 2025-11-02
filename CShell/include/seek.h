#ifndef SEEK_H
#define SEEK_H

#include "cshell.h"

// Function to handle seek command
void handle_seek_command(char **args, int arg_count);

// Helper function for recursive search
void search_recursive(const char *dir_path, const char *target, 
                     bool dirs_only, bool files_only, bool execute_flag);

// Helper function to check if path matches target
bool matches_target(const char *path, const char *target);

#endif

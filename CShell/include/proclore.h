#ifndef PROCLORE_H
#define PROCLORE_H

#include "cshell.h"

// Function to handle proclore command
void handle_proclore_command(char **args, int arg_count);

// Helper function to read process information from /proc
void print_process_info(pid_t pid);

#endif

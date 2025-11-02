#ifndef REVEAL_H
#define REVEAL_H

#include "cshell.h"

// Function to handle reveal command (like ls)
bool handle_reveal_command(char **args, int arg_count);

// Helper functions
void print_file_info(const char *path, const char *name, bool show_hidden, bool long_format);
int compare_entries(const void *a, const void *b);

#endif

#ifndef MYSHRC_H
#define MYSHRC_H

#include "cshell.h"

// Function declarations
void load_myshrc();
void add_alias(const char *alias, const char *command);
char* resolve_alias(const char *command);
void handle_alias_command(char **args, int arg_count);

#endif

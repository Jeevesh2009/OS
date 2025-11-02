#ifndef FG_BG_H
#define FG_BG_H

#include "cshell.h"

// Function declarations
void handle_fg_command(char **args, int arg_count);
void handle_bg_command(char **args, int arg_count);
void bring_to_foreground(pid_t pid);
void send_to_background(pid_t pid);

#endif

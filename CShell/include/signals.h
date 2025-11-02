#ifndef SIGNALS_H
#define SIGNALS_H

#include "cshell.h"

// Function declarations
void setup_signal_handlers();
void handle_sigint(int sig);
void handle_sigtstp(int sig);
void handle_sigchld(int sig);
void send_signal_to_process(pid_t pid, int signal);

// Global variables for signal handling
extern pid_t foreground_pid;
extern int foreground_job_id;

#endif

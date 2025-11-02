#ifndef REDIRECTION_H
#define REDIRECTION_H

#include "cshell.h"

// Structure for I/O redirection
typedef struct RedirectionInfo {
    char *input_file;
    char *output_file;
    int append_output;
    int input_fd;
    int output_fd;
} RedirectionInfo;

// Function declarations
RedirectionInfo* parse_redirection(char **args, int *arg_count);
void setup_redirection(RedirectionInfo *redir);
void cleanup_redirection(RedirectionInfo *redir);
void free_redirection(RedirectionInfo *redir);

#endif

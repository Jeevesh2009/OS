#ifndef PIPES_H
#define PIPES_H

#include "cshell.h"

typedef struct {
    char ***commands;      // Array of commands (each command is an array of strings)
    int *arg_counts;       // Number of arguments for each command
    int num_commands;      // Number of commands in the pipeline
    RedirectionInfo *redir;  // I/O redirection info for the pipeline
} PipelineInfo;

// Function declarations
PipelineInfo* parse_pipeline(char **args, int arg_count);
void execute_pipeline(PipelineInfo *pipeline, bool background);
void free_pipeline(PipelineInfo *pipeline);

#endif

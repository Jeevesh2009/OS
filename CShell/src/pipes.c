#include "../include/pipes.h"
#include "../include/redirection.h"

PipelineInfo* parse_pipeline(char **args, int arg_count) {
    PipelineInfo *pipeline = malloc(sizeof(PipelineInfo));
    pipeline->commands = NULL;
    pipeline->arg_counts = NULL;
    pipeline->num_commands = 0;
    pipeline->redir = NULL;
    
    // First, handle I/O redirection for the entire pipeline
    pipeline->redir = parse_redirection(args, &arg_count);
    
    // Count number of pipes to determine number of commands
    int pipe_count = 0;
    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_count++;
        }
    }
    pipeline->num_commands = pipe_count + 1;
    
    // Allocate memory for commands
    pipeline->commands = malloc(pipeline->num_commands * sizeof(char**));
    pipeline->arg_counts = malloc(pipeline->num_commands * sizeof(int));
    
    // Parse each command
    int cmd_idx = 0;
    int start_idx = 0;
    
    for (int i = 0; i <= arg_count; i++) {
        if (i == arg_count || strcmp(args[i], "|") == 0) {
            // End of current command
            int cmd_arg_count = i - start_idx;
            pipeline->arg_counts[cmd_idx] = cmd_arg_count;
            
            if (cmd_arg_count > 0) {
                pipeline->commands[cmd_idx] = malloc((cmd_arg_count + 1) * sizeof(char*));
                for (int j = 0; j < cmd_arg_count; j++) {
                    pipeline->commands[cmd_idx][j] = strdup(args[start_idx + j]);
                }
                pipeline->commands[cmd_idx][cmd_arg_count] = NULL;
            } else {
                pipeline->commands[cmd_idx] = NULL;
            }
            
            cmd_idx++;
            start_idx = i + 1;
        }
    }
    
    return pipeline;
}

void execute_pipeline(PipelineInfo *pipeline, bool background) {
    if (!pipeline || pipeline->num_commands == 0) {
        return;
    }
    
    // If only one command, handle it without pipes
    if (pipeline->num_commands == 1) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            setup_redirection(pipeline->redir);
            execvp(pipeline->commands[0][0], pipeline->commands[0]);
            printf("ERROR : '%s' is not a valid command\n", pipeline->commands[0][0]);
            exit(1);
        } else if (pid > 0) {
            if (background) {
                add_background_process(pid, pipeline->commands[0][0]);
                printf("[%d] %d\n", 1, pid); // Simplified job numbering
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        }
        return;
    }
    
    // Multiple commands - create pipes
    int pipes[pipeline->num_commands - 1][2];
    pid_t pids[pipeline->num_commands];
    
    // Create all pipes
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }
    
    // Create child processes
    for (int i = 0; i < pipeline->num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == 0) {
            // Child process
            
            // Handle input
            if (i == 0) {
                // First command - might have input redirection
                if (pipeline->redir && pipeline->redir->input_file) {
                    int input_fd = open(pipeline->redir->input_file, O_RDONLY);
                    if (input_fd != -1) {
                        dup2(input_fd, STDIN_FILENO);
                        close(input_fd);
                    }
                }
            } else {
                // Not first command - read from previous pipe
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Handle output
            if (i == pipeline->num_commands - 1) {
                // Last command - might have output redirection
                if (pipeline->redir && pipeline->redir->output_file) {
                    int flags = O_WRONLY | O_CREAT;
                    if (pipeline->redir->append_output) {
                        flags |= O_APPEND;
                    } else {
                        flags |= O_TRUNC;
                    }
                    int output_fd = open(pipeline->redir->output_file, flags, 0644);
                    if (output_fd != -1) {
                        dup2(output_fd, STDOUT_FILENO);
                        close(output_fd);
                    }
                }
            } else {
                // Not last command - write to next pipe
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Close all pipe file descriptors
            for (int j = 0; j < pipeline->num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Execute the command
            execvp(pipeline->commands[i][0], pipeline->commands[i]);
            printf("ERROR : '%s' is not a valid command\n", pipeline->commands[i][0]);
            exit(1);
        }
    }
    
    // Parent process - close all pipes and wait
    for (int i = 0; i < pipeline->num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    if (background) {
        // Add the last process to background processes list
        add_background_process(pids[pipeline->num_commands - 1], pipeline->commands[pipeline->num_commands - 1][0]);
        printf("[%d] %d\n", 1, pids[pipeline->num_commands - 1]); // Simplified job numbering
    } else {
        // Wait for all processes
        for (int i = 0; i < pipeline->num_commands; i++) {
            int status;
            waitpid(pids[i], &status, 0);
        }
    }
}

void free_pipeline(PipelineInfo *pipeline) {
    if (!pipeline) return;
    
    for (int i = 0; i < pipeline->num_commands; i++) {
        if (pipeline->commands[i]) {
            for (int j = 0; j < pipeline->arg_counts[i]; j++) {
                free(pipeline->commands[i][j]);
            }
            free(pipeline->commands[i]);
        }
    }
    
    free(pipeline->commands);
    free(pipeline->arg_counts);
    free_redirection(pipeline->redir);
    free(pipeline);
}

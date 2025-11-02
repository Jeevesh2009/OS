#include "../include/redirection.h"

RedirectionInfo* parse_redirection(char **args, int *arg_count) {
    RedirectionInfo *redir = malloc(sizeof(RedirectionInfo));
    redir->input_file = NULL;
    redir->output_file = NULL;
    redir->append_output = false;
    redir->input_fd = -1;
    redir->output_fd = -1;
    
    // Create new args array without redirection operators
    char **new_args = malloc((*arg_count + 1) * sizeof(char*));
    int new_count = 0;
    
    for (int i = 0; i < *arg_count; i++) {
        if (strcmp(args[i], "<") == 0) {
            // Input redirection
            if (i + 1 < *arg_count) {
                redir->input_file = strdup(args[i + 1]);
                i++; // Skip the filename
            }
        } else if (strcmp(args[i], ">") == 0) {
            // Output redirection
            if (i + 1 < *arg_count) {
                redir->output_file = strdup(args[i + 1]);
                redir->append_output = false;
                i++; // Skip the filename
            }
        } else if (strcmp(args[i], ">>") == 0) {
            // Append output redirection
            if (i + 1 < *arg_count) {
                redir->output_file = strdup(args[i + 1]);
                redir->append_output = true;
                i++; // Skip the filename
            }
        } else {
            // Regular argument
            new_args[new_count] = strdup(args[i]);
            new_count++;
        }
    }
    new_args[new_count] = NULL;
    
    // Replace original args with new args
    for (int i = 0; i < *arg_count; i++) {
        free(args[i]);
    }
    for (int i = 0; i < new_count; i++) {
        args[i] = new_args[i];
    }
    args[new_count] = NULL;
    *arg_count = new_count;
    
    free(new_args);
    return redir;
}

void setup_redirection(RedirectionInfo *redir) {
    if (redir->input_file) {
        redir->input_fd = open(redir->input_file, O_RDONLY);
        if (redir->input_fd == -1) {
            perror("Input redirection");
            return;
        }
        dup2(redir->input_fd, STDIN_FILENO);
    }
    
    if (redir->output_file) {
        int flags = O_WRONLY | O_CREAT;
        if (redir->append_output) {
            flags |= O_APPEND;
        } else {
            flags |= O_TRUNC;
        }
        
        redir->output_fd = open(redir->output_file, flags, 0644);
        if (redir->output_fd == -1) {
            perror("Output redirection");
            return;
        }
        dup2(redir->output_fd, STDOUT_FILENO);
    }
}

void cleanup_redirection(RedirectionInfo *redir) {
    if (redir->input_fd != -1) {
        close(redir->input_fd);
    }
    if (redir->output_fd != -1) {
        close(redir->output_fd);
    }
}

void free_redirection(RedirectionInfo *redir) {
    if (redir) {
        free(redir->input_file);
        free(redir->output_file);
        free(redir);
    }
}

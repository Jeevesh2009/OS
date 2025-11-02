#include "../include/tokenize.h"

// Trim leading and trailing whitespace from a string
char* trim_whitespace(char* str) {
    char* end;
    
    // Trim leading space
    while (*str == ' ' || *str == '\t') str++;
    
    // If all spaces
    if (*str == 0) return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) end--;
    
    // Write new null terminator
    *(end + 1) = 0;
    
    return str;
}

// Remove extra spaces and tabs, leaving only single spaces
void remove_extra_spaces(char* str) {
    char* src = str;
    char* dst = str;
    bool prev_space = false;
    
    while (*src) {
        if (*src == ' ' || *src == '\t') {
            if (!prev_space) {
                *dst++ = ' ';
                prev_space = true;
            }
        } else {
            *dst++ = *src;
            prev_space = false;
        }
        src++;
    }
    *dst = '\0';
}

// Parse a single command string into arguments
Command parse_command(char* cmd_str) {
    Command cmd = {0};
    char* trimmed = trim_whitespace(cmd_str);
    
    // Check if command should run in background
    int len = strlen(trimmed);
    if (len > 0 && trimmed[len - 1] == '&') {
        cmd.background = true;
        trimmed[len - 1] = '\0';  // Remove the '&'
        trimmed = trim_whitespace(trimmed);  // Trim again after removing '&'
    }
    
    if (strlen(trimmed) == 0) {
        return cmd;  // Empty command
    }
    
    // Allocate memory for arguments
    cmd.args = malloc(MAX_TOKENS * sizeof(char*));
    cmd.arg_count = 0;
    
    // Tokenize the command
    char* token = strtok(trimmed, " \t");
    while (token != NULL && cmd.arg_count < MAX_TOKENS - 1) {
        cmd.args[cmd.arg_count] = malloc(strlen(token) + 1);
        strcpy(cmd.args[cmd.arg_count], token);
        cmd.arg_count++;
        token = strtok(NULL, " \t");
    }
    
    // Null-terminate the arguments array
    cmd.args[cmd.arg_count] = NULL;
    
    return cmd;
}

// Parse input string into multiple commands separated by ';' or '&'
ParsedInput* parse_input(const char* input) {
    ParsedInput* parsed = malloc(sizeof(ParsedInput));
    parsed->command_count = 0;
    
    // Make a copy of input to work with
    char* input_copy = malloc(strlen(input) + 1);
    strcpy(input_copy, input);
    
    // Remove extra spaces and tabs
    remove_extra_spaces(input_copy);
    
    // Split by semicolon first
    char* semicolon_token = strtok(input_copy, ";");
    
    while (semicolon_token != NULL && parsed->command_count < MAX_COMMANDS) {
        // For each semicolon-separated part, we might have '&' separated commands
        char* ampersand_pos = strchr(semicolon_token, '&');
        
        if (ampersand_pos != NULL) {
            // Handle commands with '&'
            char* start = semicolon_token;
            while (start != NULL && parsed->command_count < MAX_COMMANDS) {
                char* amp_pos = strchr(start, '&');
                if (amp_pos != NULL) {
                    *amp_pos = '\0';  // Terminate the command before '&'
                    
                    // Parse the command before '&' (should run in background)
                    char* cmd_with_amp = malloc(strlen(start) + 3);  // +2 for " &" and +1 for null
                    strcpy(cmd_with_amp, start);
                    strcat(cmd_with_amp, " &");
                    
                    Command cmd = parse_command(cmd_with_amp);
                    if (cmd.arg_count > 0) {
                        parsed->commands[parsed->command_count++] = cmd;
                    }
                    free(cmd_with_amp);
                    
                    start = amp_pos + 1;  // Move to after the '&'
                } else {
                    // Last command (no more '&')
                    Command cmd = parse_command(start);
                    if (cmd.arg_count > 0) {
                        parsed->commands[parsed->command_count++] = cmd;
                    }
                    start = NULL;
                }
            }
        } else {
            // No '&' in this part, parse normally
            Command cmd = parse_command(semicolon_token);
            if (cmd.arg_count > 0) {
                parsed->commands[parsed->command_count++] = cmd;
            }
        }
        
        semicolon_token = strtok(NULL, ";");
    }
    
    free(input_copy);
    return parsed;
}

// Free memory allocated for parsed input
void free_parsed_input(ParsedInput* parsed) {
    if (parsed == NULL) return;
    
    for (int i = 0; i < parsed->command_count; i++) {
        for (int j = 0; j < parsed->commands[i].arg_count; j++) {
            free(parsed->commands[i].args[j]);
        }
        free(parsed->commands[i].args);
    }
    free(parsed);
}

// Execute a single command
void execute_single_command(Command* cmd) {
    if (cmd->arg_count == 0) return;
    
    // Resolve aliases first
    char *resolved = resolve_alias(cmd->args[0]);
    if (resolved) {
        // Replace the command with its alias
        free(cmd->args[0]);
        cmd->args[0] = strdup(resolved);
    }
    
    // Check for pipes or redirection FIRST
    bool has_pipes = false;
    bool has_redirection = false;
    
    for (int i = 0; i < cmd->arg_count; i++) {
        if (strcmp(cmd->args[i], "|") == 0) {
            has_pipes = true;
            break;
        }
        if (strcmp(cmd->args[i], "<") == 0 || strcmp(cmd->args[i], ">") == 0 || strcmp(cmd->args[i], ">>") == 0) {
            has_redirection = true;
        }
    }
    
    // If it's a pipeline or has redirection, handle it specially
    if (has_pipes) {
        // Handle pipeline - this will create child processes for ALL commands
        PipelineInfo *pipeline = parse_pipeline(cmd->args, cmd->arg_count);
        execute_pipeline(pipeline, cmd->background);
        free_pipeline(pipeline);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    } else if (has_redirection) {
        // Handle simple redirection
        RedirectionInfo *redir = parse_redirection(cmd->args, &cmd->arg_count);
        
        pid_t pid = fork();
        if (pid == 0) {
            setup_redirection(redir);
            
            // Check if it's a built-in command and handle in child process
            if (strcmp(cmd->args[0], "hop") == 0) {
                handle_hop_command(cmd->args, cmd->arg_count);
                exit(0);
            } else if (strcmp(cmd->args[0], "reveal") == 0) {
                handle_reveal_command(cmd->args, cmd->arg_count);
                exit(0);
            } else if (strcmp(cmd->args[0], "seek") == 0) {
                handle_seek_command(cmd->args, cmd->arg_count);
                exit(0);
            } else if (strcmp(cmd->args[0], "proclore") == 0) {
                handle_proclore_command(cmd->args, cmd->arg_count);
                exit(0);
            } else if (strcmp(cmd->args[0], "activities") == 0) {
                handle_activities_command(cmd->args, cmd->arg_count);
                exit(0);
            } else {
                // External command
                execvp(cmd->args[0], cmd->args);
                printf("ERROR : '%s' is not a valid command\n", cmd->args[0]);
                exit(1);
            }
        } else if (pid > 0) {
            if (cmd->background) {
                add_background_process(pid, cmd->args[0]);
                add_activity(pid, cmd->args[0]);
                printf("[%d] %d\n", 1, pid);
            } else {
                foreground_pid = pid;
                int status;
                waitpid(pid, &status, WUNTRACED);
                foreground_pid = 0;
            }
        }
        
        cleanup_redirection(redir);
        free_redirection(redir);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    // No pipes or redirection - handle built-in commands normally in parent process
    if (strcmp(cmd->args[0], "hop") == 0) {
        handle_hop_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "reveal") == 0) {
        handle_reveal_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "log") == 0) {
        handle_log_command(cmd->args, cmd->arg_count);
        // Don't add log command to log
        return;
    }
    
    if (strcmp(cmd->args[0], "proclore") == 0) {
        handle_proclore_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "seek") == 0) {
        handle_seek_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "activities") == 0) {
        handle_activities_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "fg") == 0) {
        handle_fg_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "bg") == 0) {
        handle_bg_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "neonate") == 0) {
        handle_neonate_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "iman") == 0) {
        handle_iman_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "alias") == 0) {
        handle_alias_command(cmd->args, cmd->arg_count);
        add_to_log(cmd->args, cmd->arg_count);
        return;
    }
    
    if (strcmp(cmd->args[0], "exit") == 0) {
        printf("Goodbye!\n");
        exit(0);
    }
    
    // Handle system commands (original functionality)
    execute_system_command(cmd->args, cmd->arg_count, cmd->background);
    add_to_log(cmd->args, cmd->arg_count);
}

// Execute all parsed commands
void execute_commands(ParsedInput* parsed) {
    for (int i = 0; i < parsed->command_count; i++) {
        execute_single_command(&parsed->commands[i]);
        
        // Handle background process cleanup
        handle_background_processes();
    }
}
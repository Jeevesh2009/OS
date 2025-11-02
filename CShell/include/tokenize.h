#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "cshell.h"

// Structure to hold a single command with its arguments
typedef struct {
    char **args;        // Array of arguments
    int arg_count;      // Number of arguments
    bool background;    // Whether command should run in background
} Command;

// Structure to hold all parsed commands
typedef struct {
    Command commands[MAX_COMMANDS];
    int command_count;
} ParsedInput;

// Function declarations
ParsedInput* parse_input(const char* input);
void free_parsed_input(ParsedInput* parsed);
void execute_commands(ParsedInput* parsed);
void execute_single_command(Command* cmd);
char* trim_whitespace(char* str);
void remove_extra_spaces(char* str);

#endif

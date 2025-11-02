#include "../include/myshrc.h"

typedef struct {
    char alias[64];
    char command[256];
} AliasEntry;

static AliasEntry aliases[100];
static int alias_count = 0;

void load_myshrc() {
    char myshrc_path[PATH_MAX];
    snprintf(myshrc_path, sizeof(myshrc_path), "%s/.myshrc", actual_home);
    
    FILE *file = fopen(myshrc_path, "r");
    if (!file) {
        return; // No .myshrc file exists
    }
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines and comments
        if (strlen(line) == 0 || line[0] == '#') {
            continue;
        }
        
        // Look for alias definitions: alias name="command"
        if (strncmp(line, "alias ", 6) == 0) {
            char *alias_def = line + 6;
            char *equals = strchr(alias_def, '=');
            if (equals) {
                *equals = '\0';
                char *alias_name = alias_def;
                char *command = equals + 1;
                
                // Remove quotes from command if present
                if (command[0] == '"') {
                    command++;
                    char *end_quote = strrchr(command, '"');
                    if (end_quote) {
                        *end_quote = '\0';
                    }
                }
                
                add_alias(alias_name, command);
            }
        }
    }
    
    fclose(file);
}

void add_alias(const char *alias, const char *command) {
    if (alias_count < 100) {
        strncpy(aliases[alias_count].alias, alias, 63);
        aliases[alias_count].alias[63] = '\0';
        strncpy(aliases[alias_count].command, command, 255);
        aliases[alias_count].command[255] = '\0';
        alias_count++;
    }
}

char* resolve_alias(const char *command) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(aliases[i].alias, command) == 0) {
            return aliases[i].command;
        }
    }
    return NULL; // No alias found
}

void handle_alias_command(char **args, int arg_count) {
    if (arg_count == 1) {
        // Print all aliases
        for (int i = 0; i < alias_count; i++) {
            printf("alias %s=\"%s\"\n", aliases[i].alias, aliases[i].command);
        }
    } else if (arg_count == 2) {
        // Check if it's an alias definition
        char *equals = strchr(args[1], '=');
        if (equals) {
            *equals = '\0';
            char *alias_name = args[1];
            char *command = equals + 1;
            
            // Remove quotes if present
            if (command[0] == '"') {
                command++;
                char *end_quote = strrchr(command, '"');
                if (end_quote) {
                    *end_quote = '\0';
                }
            }
            
            add_alias(alias_name, command);
            printf("Alias added: %s=\"%s\"\n", alias_name, command);
        } else {
            // Show specific alias
            char *resolved = resolve_alias(args[1]);
            if (resolved) {
                printf("alias %s=\"%s\"\n", args[1], resolved);
            } else {
                printf("alias: %s: not found\n", args[1]);
            }
        }
    } else {
        printf("Usage: alias [name[=value]]\n");
    }
}

#include "include/cshell.h"
#include <unistd.h>

// Global variables definition
char prev_dir[PATH_MAX] = "";
char shell_home[PATH_MAX] = "";
char actual_home[PATH_MAX] = "";

int main() {
    char input[SHELL_MAX_INPUT];
    char cwd[PATH_MAX], sysname[HOST_NAME_MAX];
    struct passwd *pw = getpwuid(getuid());
    const char *username = pw ? pw->pw_name : "user";

    // Get system name
    gethostname(sysname, sizeof(sysname));

    // Get actual home directory from environment or passwd
    const char *home_env = getenv("HOME");
    if (home_env) {
        strcpy(actual_home, home_env);
    } else {
        strcpy(actual_home, pw ? pw->pw_dir : "/");
    }

    // Get shell's starting directory (shell home)
    if (!getcwd(shell_home, sizeof(shell_home))) {
        perror("getcwd");
        exit(1);
    }

    // Initialize log system
    initialize_log();
    
    // Setup signal handlers
    setup_signal_handlers();
    
    // Load .myshrc file
    load_myshrc();

    printf("Hello, CShell!\n");
    printf("Welcome to the CShell command line interface.\n");

    while (true) {
        if (!getcwd(cwd, sizeof(cwd))) {
            perror("getcwd");
            exit(1);
        }
        print_prompt(username, sysname, shell_home, cwd);

        if (fgets(input, SHELL_MAX_INPUT, stdin) == NULL) {
            printf("\n");
            break;
        }
        input[strcspn(input, "\n")] = 0;
        
        // Skip empty input
        if (strlen(trim_whitespace(input)) == 0) {
            continue;
        }
        
        // Parse and execute the input
        ParsedInput* parsed = parse_input(input);
        execute_commands(parsed);
        free_parsed_input(parsed);
    }

    return 0;
}
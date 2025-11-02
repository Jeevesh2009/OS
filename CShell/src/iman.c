#include "../include/iman.h"

void handle_iman_command(char **args, int arg_count) {
    if (arg_count != 2) {
        printf("Usage: iman <command_name>\n");
        return;
    }
    
    fetch_man_page(args[1]);
}

void fetch_man_page(const char *command) {
    // For now, we'll use a simple approach with wget or curl if available
    char cmd[512];
    
    // First try with curl
    snprintf(cmd, sizeof(cmd), "curl -s \"https://man.he.net/?topic=%s&section=all\" 2>/dev/null | grep -A 50 \"<PRE>\" | grep -B 50 \"</PRE>\" | sed 's/<[^>]*>//g' | head -50", command);
    
    int result = system(cmd);
    if (result != 0) {
        // If curl fails, try wget
        snprintf(cmd, sizeof(cmd), "wget -qO- \"https://man.he.net/?topic=%s&section=all\" 2>/dev/null | grep -A 50 \"<PRE>\" | grep -B 50 \"</PRE>\" | sed 's/<[^>]*>//g' | head -50", command);
        
        result = system(cmd);
        if (result != 0) {
            printf("ERROR: Could not fetch man page for '%s'\n", command);
            printf("Please install curl or wget to use iman command\n");
        }
    }
}

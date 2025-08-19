#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>

#define FILENAME "newfile.txt"

int main() {
    char operation[100];

    // Create file with 0644 permissions
    int fd = open(FILENAME, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error creating file");
        exit(1);
    }
    close(fd);

    printf("File Management Started......\n");

    while (true) {
        printf("Enter command (INPUT/PRINT/STOP): ");
        if (scanf("%99s", operation) != 1) continue;

        if (strcmp(operation, "STOP") == 0) {
            break;
        }
        else if (strcmp(operation, "INPUT") == 0) {
            getchar(); // consume leftover newline
            char input[256];
            printf("-> ");
            if (fgets(input, sizeof(input), stdin) == NULL) continue;

            // remove trailing newline
            input[strcspn(input, "\n")] = '\0';

            fd = open(FILENAME, O_WRONLY | O_APPEND);
            if (fd == -1) {
                perror("Error opening file for append");
                continue;
            }

            write(fd, input, strlen(input));
            write(fd, "\n", 1);
            close(fd);
        }
        else if (strcmp(operation, "PRINT") == 0) {
            char buffer[256];
            fd = open(FILENAME, O_RDONLY);
            if (fd == -1) {
                perror("Error opening file for read");
                continue;
            }

            int bytesRead;
            while ((bytesRead = read(fd, buffer, sizeof(buffer)-1)) > 0) {
                buffer[bytesRead] = '\0';  // null terminate
                printf("%s", buffer);
            }
            close(fd);
        }
        else {
            printf("Invalid operation.\n");
        }
    }

    return 0;
}

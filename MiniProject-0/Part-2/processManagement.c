#define _GNU_SOURCE
#include <stdio.h>      // printf
#include <stdlib.h>     // exit
#include <unistd.h>     // fork, exec, getpid, getppid, read, write, close, sleep
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include <fcntl.h>      // open
#include <sys/stat.h>   // file modes
#include <string.h>     // strlen, strcmp, strncmp, memset
#include <errno.h>      // errno

#define FILENAME "newfile.txt"

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static ssize_t write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char *)buf;
    size_t left = n;
    while (left > 0) {
        ssize_t w = write(fd, p, left);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        left -= (size_t)w;
        p    += w;
    }
    return (ssize_t)n;
}

static int write_textfile_overwrite(const char *path, const char *text) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) return -1;
    size_t len = strlen(text);
    if (write_all(fd, text, len) == -1) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    if (close(fd) == -1) return -1;
    return 0;
}

static int read_entire_file(const char *path, char *buf, size_t cap) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) return -1;
    size_t off = 0;
    while (off + 1 < cap) {
        ssize_t r = read(fd, buf + off, cap - 1 - off);
        if (r < 0) {
            if (errno == EINTR) continue;
            int e = errno;
            close(fd);
            errno = e;
            return -1;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    buf[off] = '\0';
    if (close(fd) == -1) return -1;
    return (int)off; // bytes read
}


static int writer_mode(void) {
    // Child (post-exec) writes its PARENT PID into newfile.txt
    pid_t ppid = getppid();
    char line[128];
    int n = snprintf(line, sizeof(line), "Parent PID recorded by exec'd child: %d\n", (int)ppid);
    if (n < 0 || (size_t)n >= sizeof(line)) {
        fprintf(stderr, "snprintf failed in writer_mode\n");
        return EXIT_FAILURE;
    }
    if (write_textfile_overwrite(FILENAME, line) == -1) {
        perror("writer_mode: write_textfile_overwrite");
        return EXIT_FAILURE;
    }
    // Optional: say something on stdout (not part of file)
    printf("[writer] Wrote to %s: %s", FILENAME, line);
    return EXIT_SUCCESS;
}


static void task1_variable_inheritance(void) {
    printf("\n===== Task 1: Variable Inheritance =====\n");
    int x = 25;
    printf("[parent %d] initial x = %d\n", (int)getpid(), x);

    pid_t pid = fork();
    if (pid < 0) die("fork (task1)");

    if (pid == 0) {
        // Child
        printf("[child  %d] received x = %d\n", (int)getpid(), x);
        x += 10;
        printf("[child  %d] modified x = %d\n", (int)getpid(), x);
        _exit(0);
    } else {
        // Parent
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) die("waitpid (task1)");
        x -= 5;
        printf("[parent %d] modified x = %d (independent of child)\n", (int)getpid(), x);
        printf("Observation: parent & child have separate copies (copy-on-write).\n");
    }
}


static void task2_exec_and_write_parent_pid(void) {
    printf("\n===== Task 2: Child exec() writes parent PID to file =====\n");

    pid_t pid = fork();
    if (pid < 0) die("fork (task2)");

    if (pid == 0) {
        // Child → replace itself with "writer mode" of the same binary.
        // Using /proc/self/exe ensures path to current executable.
        execl("/proc/self/exe", "processManagement", "--writer", (char *)NULL);
        // If we reach here, exec failed.
        perror("exec failed (task2)");
        _exit(127);
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) die("waitpid (task2)");
        if (WIFEXITED(status)) {
            printf("[parent %d] writer child exited with code %d\n",
                   (int)getpid(), WEXITSTATUS(status));
        } else {
            printf("[parent %d] writer child terminated abnormally\n", (int)getpid());
        }

        // Show file contents for sanity
        char buf[512];
        int r = read_entire_file(FILENAME, buf, sizeof(buf));
        if (r >= 0) {
            printf("Contents of %s after Task-2:\n%s", FILENAME, buf);
        } else {
            perror("read_entire_file (task2)");
        }
    }
}

/* ======================================
   Task 3: Orphan child prints new PPID
   ====================================== */

static void task3_orphan_and_observe(void) {
    printf("\n===== Task 3: Orphan Process Observation =====\n");

    pid_t pid = fork();
    if (pid < 0) die("fork (task3)");

    if (pid == 0) {
        // Child: wait a bit so parent can exit
        sleep(1);
        pid_t mypid  = getpid();
        pid_t myppid = getppid();
        printf("[child  %d] post-orphan getppid() = %d (likely 1: init/systemd)\n",
               (int)mypid, (int)myppid);

        // For comparison, print what Task-2 wrote
        char buf[512];
        int r = read_entire_file(FILENAME, buf, sizeof(buf));
        if (r >= 0) {
            printf("[child  %d] Previously recorded in %s:\n%s",
                   (int)mypid, FILENAME, buf);
        } else {
            perror("read_entire_file (task3)");
        }
        _exit(0);
    } else {
        // Parent exits immediately → child becomes orphan
        printf("[parent %d] exiting immediately to orphan the child %d\n",
               (int)getpid(), (int)pid);
        // No wait() on purpose.
        // Use _exit to avoid stdio buffering surprises.
        _exit(0);
    }
}


int main(int argc, char **argv) {
    // Special exec-path (Task-2): act as "writer"
    if (argc >= 2 && strcmp(argv[1], "--writer") == 0) {
        int rc = writer_mode();
        return rc;
    }

    printf("=== Mini Project 0: Part 2 (Process Creation & Management) ===\n");

    // Task 1
    task1_variable_inheritance();

    // Task 2
    task2_exec_and_write_parent_pid();

    // Task 3 (parent will exit; child will finish printing)
    task3_orphan_and_observe();

    return 0;
}

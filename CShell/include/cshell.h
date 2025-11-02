#ifndef CSHELL_H
#define CSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>

// Forward declarations to resolve circular dependencies
typedef struct RedirectionInfo RedirectionInfo;

#define SHELL_MAX_INPUT 1024
#define MAX_TOKENS 100
#define MAX_COMMANDS 50
#define PATH_MAX 4096

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

// Global variables
extern char prev_dir[PATH_MAX];
extern char shell_home[PATH_MAX];
extern char actual_home[PATH_MAX];

// Include all specification headers
#include "display.h"
#include "tokenize.h"
#include "hop.h"
#include "reveal.h"
#include "log.h"
#include "system.h"
#include "proclore.h"
#include "seek.h"
#include "redirection.h"
#include "pipes.h"
#include "activities.h"
#include "signals.h"
#include "fg_bg.h"
#include "neonate.h"
#include "iman.h"
#include "myshrc.h"

#endif

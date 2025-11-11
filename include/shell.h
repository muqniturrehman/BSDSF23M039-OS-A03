#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define MAX_JOBS 100

struct job {
    int id;
    pid_t pid;
    char command[256];
    int active;
};

extern struct job jobs[MAX_JOBS];
extern int job_count;

void add_job(pid_t pid, char *cmdline);
void check_jobs();

extern char* history[];
extern int history_count;
#define HISTORY_SIZE 20

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);
int handle_builtin(char **args);
int execute_pipe(char ***cmds);

#endif // SHELL_H

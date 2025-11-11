#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit
#include <string.h>     // strcmp
#include <unistd.h>     // fork, execvp, dup2, close
#include <fcntl.h>      // open, O_* flags
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include "shell.h"

struct job jobs[MAX_JOBS];
int job_count = 0;
int execute(char* arglist[]) {
    int status;
    pid_t cpid;
    int background = 0;

    // Check if last argument is "&"
    int i = 0;
    while (arglist[i] != NULL) i++;
    if (i > 0 && strcmp(arglist[i - 1], "&") == 0) {
        background = 1;
        arglist[i - 1] = NULL; // remove "&"
    }

    cpid = fork();

    if (cpid == -1) {
        perror("fork failed");
        return 1;
    }

    if (cpid == 0) { // child
        execvp(arglist[0], arglist);
        perror("exec failed");
        exit(1);
    } 
    else { // parent
        if (background) {
			add_job(cpid, arglist[0]);
            printf("[BG] Process running in background (PID: %d)\n", cpid);
        } else {
            waitpid(cpid, &status, 0);
        }
    }

    return 0;
}

int execute_pipe(char ***cmds) {
    int fd[2];
    int in_fd = 0; // first command reads from stdin
    pid_t pid;
    int i = 0;

    while (cmds[i] != NULL) {
        pipe(fd);
        pid = fork();

        if (pid == 0) {                     // child
            dup2(in_fd, STDIN_FILENO);      // previous input
            if (cmds[i+1] != NULL)
                dup2(fd[1], STDOUT_FILENO); // next output
            close(fd[0]);
            execvp(cmds[i][0], cmds[i]);
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {               // parent
            wait(NULL);
            close(fd[1]);
            in_fd = fd[0];                  // next input = this pipe's read end
            i++;
        } else {
            perror("fork");
            return 1;
        }
    }
    return 0;
}
void add_job(pid_t pid, char *cmdline) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].id = job_count + 1;
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmdline, sizeof(jobs[job_count].command));
        jobs[job_count].active = 1;
        job_count++;
    }
}

void check_jobs() {
    int status;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active && waitpid(jobs[i].pid, &status, WNOHANG) > 0) {
            jobs[i].active = 0;
            printf("[DONE] %d  %s\n", jobs[i].pid, jobs[i].command);
        }
    }
}
#include <signal.h>
#include <termios.h>

void fg_command(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("fg: invalid job id\n");
        return;
    }

    struct job *j = &jobs[job_id - 1];
    if (!j->active) {
        printf("fg: job [%d] already finished\n", job_id);
        return;
    }

    printf("Bringing [%d] %s to foreground\n", j->pid, j->command);
    tcsetpgrp(STDIN_FILENO, j->pid);     // give terminal control
    kill(j->pid, SIGCONT);               // resume if stopped
    waitpid(j->pid, NULL, 0);            // wait for completion
    tcsetpgrp(STDIN_FILENO, getpgrp());  // restore shell control
    j->active = 0;
}

void bg_command(int job_id) {
    if (job_id <= 0 || job_id > job_count) {
        printf("bg: invalid job id\n");
        return;
    }

    struct job *j = &jobs[job_id - 1];
    if (!j->active) {
        printf("bg: job [%d] already finished\n", job_id);
        return;
    }

    printf("Resuming [%d] %s in background\n", j->pid, j->command);
    kill(j->pid, SIGCONT);
}

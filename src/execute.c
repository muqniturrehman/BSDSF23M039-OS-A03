#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit
#include <string.h>     // strcmp
#include <unistd.h>     // fork, execvp, dup2, close
#include <fcntl.h>      // open, O_* flags
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // waitpid
#include "shell.h"
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

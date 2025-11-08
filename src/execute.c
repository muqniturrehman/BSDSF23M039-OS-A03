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
    pid_t cpid = fork();

    if (cpid == 0) { // child
        // check for redirection
        for (int i = 0; arglist[i] != NULL; i++) {

            // Output redirection >
            if (strcmp(arglist[i], ">") == 0) {
                int fd = open(arglist[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("open"); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
                arglist[i] = NULL; // cut off from execvp
                break;
            }

            // Input redirection <
            if (strcmp(arglist[i], "<") == 0) {
                int fd = open(arglist[i+1], O_RDONLY);
                if (fd < 0) { perror("open"); exit(1); }
                dup2(fd, STDIN_FILENO);
                close(fd);
                arglist[i] = NULL;
                break;
            }
        }

        execvp(arglist[0], arglist);
        perror("exec failed");
        exit(1);
    } else if (cpid > 0) {
        waitpid(cpid, &status, 0);
    } else {
        perror("fork failed");
    }
    return 0;
}

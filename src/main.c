#include "shell.h"

#define HISTORY_SIZE 20
char* history[HISTORY_SIZE];
int history_count = 0;

int main() {
    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        /* -----------------------------
           STEP 1: Store command in history
        ------------------------------ */
        if (cmdline && strlen(cmdline) > 0) {
            if (history_count < HISTORY_SIZE) {
                history[history_count++] = strdup(cmdline);
            } else {
                free(history[0]);
                for (int i = 1; i < HISTORY_SIZE; i++)
                    history[i - 1] = history[i];
                history[HISTORY_SIZE - 1] = strdup(cmdline);
            }
        }

        /* -----------------------------
           STEP 2: Check for !n re-execution
        ------------------------------ */
        if (cmdline[0] == '!' && strlen(cmdline) > 1) {
            int n = atoi(cmdline + 1);
            if (n > 0 && n <= history_count) {
                free(cmdline);
                cmdline = strdup(history[n - 1]);
                printf("%s\n", cmdline); // echo the command being re-run
            } else {
                printf("Invalid history number.\n");
                free(cmdline);
                continue;
            }
        }

        /* -----------------------------
           STEP 3: Tokenize and run
        ------------------------------ */
        if ((arglist = tokenize(cmdline)) != NULL) {

            if (handle_builtin(arglist) == 0)
                execute(arglist);

            // free allocated memory from tokenize
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}

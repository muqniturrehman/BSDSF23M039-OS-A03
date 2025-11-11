#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    int c, pos = 0;

    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL; // Handle Ctrl+D
    }
    
    cmdline[pos] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    // Edge case: empty command line
    if (cmdline == NULL || cmdline[0] == '\0' || cmdline[0] == '\n') {
        return NULL;
    }

    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int i = 0; i < MAXARGS + 1; i++) {
        arglist[i] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[i], ARGLEN);
    }

    char* cp = cmdline;
    char* start;
    int len;
    int argnum = 0;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++; // Skip leading whitespace
        
        if (*cp == '\0') break; // Line was only whitespace

        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t')) {
            len++;
        }
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }

    if (argnum == 0) { // No arguments were parsed
        for(int i = 0; i < MAXARGS + 1; i++) free(arglist[i]);
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;
    return arglist;
}


int handle_builtin(char **args) {
    if (args[0] == NULL)
        return 1;  // empty command, do nothing
	    
		
	// ======== history ========
	if (strcmp(args[0], "history") == 0) {
    for (int i = 0; i < history_count; i++)
        printf("%d  %s\n", i + 1, history[i]);
    return 1;
}

    // ======== exit ========
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting MyShell...\n");
        exit(0);   // terminate shell process
    }

    // ======== cd ========
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (chdir(args[1]) != 0) {
            perror("cd failed");
        }
        return 1;  // handled successfully
    }

    // ======== help ========
    if (strcmp(args[0], "help") == 0) {
        printf("Available built-in commands:\n");
        printf("  cd <directory>   - Change the current directory\n");
        printf("  help             - Display this help message\n");
        printf("  jobs             - List background jobs (placeholder)\n");
        printf("  exit             - Exit the shell\n");
        return 1;
    }

    // ======== jobs ========
    if (strcmp(args[0], "jobs") == 0) {
    check_jobs();
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].active)
            printf("[%d] %d  %s\n", jobs[i].id, jobs[i].pid, jobs[i].command);
    }
    return 1;
}

	if (strcmp(args[0], "fg") == 0) {
    if (args[1] && args[1][0] == '%')
        fg_command(atoi(args[1] + 1));
    else
        printf("Usage: fg %%<job_id>\n");
    return 1;
}

	if (strcmp(args[0], "bg") == 0) {
    if (args[1] && args[1][0] == '%')
        bg_command(atoi(args[1] + 1));
    else
        printf("Usage: bg %%<job_id>\n");
    return 1;
}

    // ======== not a built-in ========
    return 0;
}

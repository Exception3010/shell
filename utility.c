#include "myshell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>

// Initialize shell components
void initialize_shell() {
    signal(SIGINT, handle_signal);
    signal(SIGTSTP, handle_signal);
    
    for (int i = 0; i < HISTORY_SIZE; i++) {
        command_history[i] = NULL;
    }
}

// Clean up shell resources
void cleanup_shell() {
    for (int i = 0; i < HISTORY_SIZE && command_history[i] != NULL; i++) {
        free(command_history[i]);
    }
}

// Execute a command
void execute_command(char *input) {
    char *args[MAX_ARGS];
    int background = 0;
    int arg_count = parse_command(input, args, &background);
    
    if (arg_count == 0) return;
    
    if (is_internal_command(args[0])) {
        execute_internal_command(args, arg_count);
    } else {
        execute_external_command(args, arg_count, background);
    }
}

// Parse command into arguments
int parse_command(char *input, char **args, int *background) {
    char input_copy[MAX_INPUT];
    strcpy(input_copy, input);
    
    *background = 0;
    char *ampersand = strrchr(input_copy, '&');
    if (ampersand) {
        *background = 1;
        *ampersand = '\0';
        trim_whitespace(input_copy);
    }
    
    char *saveptr;
    char *token = __strtok_r(input_copy, " \t", &saveptr);
    int count = 0;
    
    while (token && count < MAX_ARGS - 1) {
        args[count++] = token;
        token = __strtok_r(NULL, " \t", &saveptr);
    }
    args[count] = NULL;
    
    return count;
}

// Check if command is internal
int is_internal_command(const char *command) {
    const char *internal_commands[] = {
        "cd", "echo", "dir", "clr", "pause", 
        "environ", "help", "history", "quit", "exit", NULL
    };
    
    for (int i = 0; internal_commands[i]; i++) {
        if (strcmp(command, internal_commands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Execute internal command
void execute_internal_command(char **args, int count) {
    char *output_file = NULL;
    int append_mode = 0;
    int saved_stdout = -1;
    
    handle_redirection(args, count, NULL, &output_file, &append_mode);
    if (output_file) {
        saved_stdout = setup_output_redirection(output_file, append_mode);
    }
    
    if (strcmp(args[0], "cd") == 0) {
        handle_cd(args, count);
    } else if (strcmp(args[0], "echo") == 0) {
        handle_echo(args, count);
    } else if (strcmp(args[0], "dir") == 0) {
        handle_dir(args, count);
    } else if (strcmp(args[0], "clr") == 0) {
        system("clear");
    } else if (strcmp(args[0], "pause") == 0) {
        handle_pause();
    } else if (strcmp(args[0], "environ") == 0) {
        handle_environ();
    } else if (strcmp(args[0], "help") == 0) {
        handle_help();
    } else if (strcmp(args[0], "history") == 0) {
        print_history();
    }
    
    if (saved_stdout != -1) {
        restore_stdout(saved_stdout);
    }
}

// Execute external command
void execute_external_command(char **args, int count, int background) {
    pid_t pid = fork();
    
    if (pid == 0) {
        char *input_file = NULL;
        char *output_file = NULL;
        int append_mode = 0;
        
        handle_redirection(args, count, &input_file, &output_file, &append_mode);
        setup_redirection(input_file, output_file, append_mode);
        
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        perror("fork");
    }
}

// ... (Include all the other utility functions from the previous implementation)
// (handle_cd, handle_echo, handle_dir, handle_pause, handle_environ, handle_help)
// (handle_redirection, setup_redirection, setup_output_redirection, restore_stdout)
// (add_to_history, print_history, trim_whitespace, handle_signal)
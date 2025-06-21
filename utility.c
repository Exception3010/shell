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
#include <ctype.h>

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
    char input_copy[INPUT_SIZE];
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

char *command_history[HISTORY_SIZE];

void handle_cd(char **args, int count) {
    if (count < 2) {
        char cwd[MAX_PATH];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s\n", cwd);
        }
        return;
    }
    if (chdir(args[1]) != 0) {
        perror("cd");
    }
}

void handle_echo(char **args, int count) {
    for (int i = 1; i < count; i++) {
        printf("%s", args[i]);
        if (i < count - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

void handle_dir(char **args, int count) {
    const char *path = (count > 1) ? args[1] : ".";
    DIR *dir = opendir(path);
    if (!dir) {
        perror("dir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
}

void handle_pause() {
    printf("Press Enter to continue...");
    fflush(stdout);
    while (getchar() != '\n');
}

extern char **environ;
void handle_environ() {
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
}

void handle_help() {
    FILE *f = fopen("README.md", "r");
    if (!f) {
        perror("help");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        fputs(line, stdout);
    }
    fclose(f);
}

void handle_redirection(char **args, int count, char **input_file, char **output_file, int *append_mode) {
    if (input_file) *input_file = NULL;
    if (output_file) *output_file = NULL;
    if (append_mode) *append_mode = 0;

    for (int i = 0; i < count; i++) {
        if (!args[i]) continue;
        if (strcmp(args[i], "<") == 0 && i + 1 < count) {
            if (input_file) *input_file = args[i+1];
            args[i] = NULL;
            for (int j = i + 2; j <= count; j++) args[j-2] = args[j];
            count -= 2; i--;
        } else if ((strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) && i + 1 < count) {
            if (output_file) *output_file = args[i+1];
            if (append_mode) *append_mode = (strcmp(args[i], ">>") == 0);
            args[i] = NULL;
            for (int j = i + 2; j <= count; j++) args[j-2] = args[j];
            count -= 2; i--;
        }
    }
}

void setup_redirection(char *input_file, char *output_file, int append_mode) {
    if (input_file) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror(input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }
    if (output_file) {
        int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
        int fd = open(output_file, flags, 0644);
        if (fd < 0) {
            perror(output_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

int setup_output_redirection(const char *output_file, int append_mode) {
    int saved = dup(STDOUT_FILENO);
    int flags = O_WRONLY | O_CREAT | (append_mode ? O_APPEND : O_TRUNC);
    int fd = open(output_file, flags, 0644);
    if (fd < 0) {
        perror(output_file);
        return saved;
    }
    dup2(fd, STDOUT_FILENO);
    close(fd);
    return saved;
}

void restore_stdout(int saved_fd) {
    dup2(saved_fd, STDOUT_FILENO);
    close(saved_fd);
}

void add_to_history(const char *cmd) {
    if (!cmd || !*cmd) return;
    free(command_history[HISTORY_SIZE-1]);
    for (int i = HISTORY_SIZE-1; i > 0; i--) {
        command_history[i] = command_history[i-1];
    }
    command_history[0] = strdup(cmd);
}

void print_history() {
    for (int i = 0; i < HISTORY_SIZE && command_history[i]; i++) {
        printf("%d %s\n", i+1, command_history[i]);
    }
}

void trim_whitespace(char *str) {
    if (!str) return;
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end-1))) end--;
    memmove(str, start, end - start);
    str[end - start] = '\0';
}

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTSTP) {
        printf("\n");
    }
}

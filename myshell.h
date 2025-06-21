#ifndef MYSHELL_H
#define MYSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#define MAX_ARGS 64
#define MAX_PATH 1024
#define INPUT_SIZE 1024
#define HISTORY_SIZE 100

extern char *command_history[HISTORY_SIZE];

// Function prototypes
void initialize_shell();
void cleanup_shell();
void execute_command(char *input);
int parse_command(char *input, char **args, int *background);
int is_internal_command(const char *command);
void execute_internal_command(char **args, int count);
void execute_external_command(char **args, int count, int background);
void handle_cd(char **args, int count);
void handle_echo(char **args, int count);
void handle_dir(char **args, int count);
void handle_pause();
void handle_environ();
void handle_help();
void print_history();
void handle_redirection(char **args, int count, char **input_file, char **output_file, int *append_mode);
void setup_redirection(char *input_file, char *output_file, int append_mode);
int setup_output_redirection(const char *output_file, int append_mode);
void restore_stdout(int saved_fd);
void trim_whitespace(char *str);
void handle_signal(int sig);
#endif


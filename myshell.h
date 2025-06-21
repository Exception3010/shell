#ifndef MYSHELL_H
#defin MYSHELL_H

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

// Function prototypes
void execute_command(char **args, int background);
void parse_redirection(char **args, int *input_fd, int *output_fd, int *append);
void internal_cd(char **args);
void internal_clr();
void internal_dir(char **args);
void internal_environ();
void internal_echo(char **args);
void internal_help();
void internal_pause();
void internal_quit();
void show_pwd();
#endif
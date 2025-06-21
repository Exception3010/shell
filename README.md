# MyShell - Custom Shell Implementation

## Overview
MyShell is a basic command-line shell implemented in C. It supports execution of internal and external commands, input/output redirection, background processing, and batch mode via a script file.

## Features
- Command prompt showing current working directory
- Internal commands like `cd`, `clr`, `dir`, `environ`, `echo`, `help`, `pause`, `quit`
- Redirection support: `>`, `>>`, `<`
- Background execution using `&`
- Batch file support

## Internal Command Descriptions

### `cd [dir]`
Changes the current working directory to `dir`. If no argument is given, prints the current directory.

### `clr`
Clears the terminal screen using ANSI escape sequences.

### `dir [dir]`
Lists the contents of the specified directory. Defaults to current directory if none is given.

### `environ`
Displays all environment variables available to the shell.

### `echo [args...]`
Displays the arguments passed to it, separated by spaces.

### `help`
Displays a help file named `readme` using the `more` command.

### `pause`
Pauses shell execution until the user presses Enter.

### `quit`
Exits the shell.

## File: main()
Initializes the shell, sets the shell path environment variable, checks for batch mode, loops over command input, and dispatches commands to execution.

## Function: execute_command()
Handles the parsing and execution of both internal and external commands. Supports:
- Redirection (`>`, `>>`, `<`)
- Background execution (`&`)
- Setting the `parent` environment variable

## Function: internal_cd()
Handles changing the directory using `chdir()` and updates `PWD` environment variable.

## Function: internal_clr()
Clears the screen using escape codes `[2J[H`.

## Function: internal_dir()
Opens the specified directory (or current one), and prints its contents using `readdir()`.

## Function: internal_environ()
Iterates over `environ` to print all environment variables.

## Function: internal_echo()
Prints all arguments passed to it starting from the second one.

## Function: internal_help()
Uses `more` to display the `readme` file content. Forks a process to run `more` and redirects the help file as standard input.

## Function: internal_pause()
Prompts the user to press Enter and waits for it using `getchar()`.

## Function: internal_quit()
Simply calls `exit(EXIT_SUCCESS)` to terminate the shell.

## Function: show_pwd()
Prints the `PWD` environment variable to stdout.

## Dependencies
- `stdio.h`, `stdlib.h`, `string.h`, `unistd.h`
- `fcntl.h`, `sys/wait.h`, `sys/stat.h`, `dirent.h`

## Compilation
```bash
gcc -o myshell myshell.c
```

## Usage
```bash
./myshell             # Interactive mode
./myshell script.txt  # Batch mode using script file
```

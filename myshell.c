#include "myshell.h"


int main(int argc, char *argv[]) {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    char cwd[MAX_PATH];
    FILE *batch_file = NULL;
    
    // Set shell environment variable
    char shell_path[MAX_PATH];
    realpath(argv[0], shell_path);
    setenv("shell", shell_path, 1);
    
    // Check for batch file mode
    if (argc > 1) {
        batch_file = fopen(argv[1], "r");
        if (!batch_file) {
            perror("Error opening batch file");
            exit(EXIT_FAILURE);
        }
    }
    
    // Main shell loop
    while (1) {
        // Get current directory for prompt
        getcwd(cwd, sizeof(cwd));
        
        // Read input
        if (batch_file) {
            if (fgets(input, sizeof(input), batch_file) == NULL) {
                break; // EOF reached
            }
            printf("%s$ %s", cwd, input);
        } else {
            printf("%s$ ", cwd);
            if (fgets(input, sizeof(input), stdin) == NULL) {
                break; // EOF (Ctrl+D) pressed
            }
        }
        
        // Parse input
        char *token = strtok(input, " \t\n");
        int i = 0;
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[i] = NULL;
        
        // Skip empty lines
        if (args[0] == NULL) {
            continue;
        }
        
        // Check for background execution
        int background = 0;
        if (i > 0 && strcmp(args[i-1], "&") == 0) {
            background = 1;
            args[i-1] = NULL;
        }
        
        // Execute command
        execute_command(args, background);
    }
    
    if (batch_file) fclose(batch_file);
    return EXIT_SUCCESS;
}
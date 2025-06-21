#include "myshell.h"


int main(int argc, char *argv[]) {
    char input[INPUT_SIZE];
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
        
        trim_whitespace(input);
        if (input[0] == '\0') {
            continue;
        }

        add_to_history(input);
        execute_command(input);
    }
    
    if (batch_file) fclose(batch_file);
    return EXIT_SUCCESS;
}
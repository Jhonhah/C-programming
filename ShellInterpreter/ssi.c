#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>


// buffer for handling ctrl+c (source: AI assistant)
sigjmp_buf prompt_buf; 

// Global variables for background process management
void handle_sigint(int sig) {
    printf("\n");  // move to a new line
    siglongjmp(prompt_buf, 1);  // jump back to prompt
}

int bg_count = 0;  // Count of background processes
int bg_cap = 10; // Initial capacity for background processes
char **arguments = NULL; // Array to store the commands for background processes
int *bg_pids = NULL; // Array to store the process IDs of background processes

// Function prototypes
int tokenize(char *,  char ***);
int create_prompt(char **);
void change_directory(char **);
void add_background_process(char **);
void check_background_processes();
void list_background_processes();

int main(int argc, char *argv[]) {
    // Initializing variables
    char **tokens, *prompt, *line;
    tokens = NULL;
    prompt = NULL; 
    line = NULL; 
    int pid, num_tokens;

    // Set up signal handling for SIGINT (source: AI assistant) 
    signal(SIGINT, handle_sigint);  

    // Initialize the prompt buffer for sigsetjmp (source: AI assistant)
    if (sigsetjmp(prompt_buf, 1) != 0) {
        // Return here if Ctr-C caught the signal
        if (prompt) {
            free(prompt);
            prompt = NULL;
        }
        if (line) {
            free(line);
            line = NULL;
        }
        if (tokens) {
            for (int i = 0; i < num_tokens; i++) {
                free(tokens[i]);
                tokens[i] = NULL; 
            }
            free(tokens);
            tokens = NULL;
        }
    }
 
   //Main loop for shell
   while (1) {
        //prompt for input
        create_prompt(&prompt); 
        line = readline(prompt);
        if (line == NULL) {
            perror("readline failed");
            free(prompt);
            exit(1);
        } 

        // exit if user types "exit"
        if (strcmp(line, "exit") == 0) {
            free(prompt);
            free(line);
            exit(0); // 
        }

        // get the tokens from the line
        num_tokens = tokenize(line, &tokens);

        //change directory if the first token is "cd"
        if (strcmp(tokens[0], "cd") == 0) {
            // Handling improper usage of cd command
            if (num_tokens != 2){
                perror("Poor Usage: cd requires exactly one argument");
                free(prompt);
                free(line);
                free(tokens[0]);
                free(tokens[1]);
                free(tokens); 
                tokens = NULL; 
                continue; 
            }
            change_directory(&tokens[1]);  // Change directory
            free(prompt);
            free(line);
            free(tokens[0]); 
            free(tokens); 
            tokens = NULL; 
            prompt = NULL; 
            line = NULL;
            continue; // Skip to the next iteration 
        }
        // Handle background processes if the first token is "bg"
        if (strcmp(tokens[0], "bg") == 0) {
            // Handling improper usage of bg command
            if (num_tokens < 2) {
                perror("Poor Usage: bg requires at least one argument");
                free(prompt);
                free(line);
                for (int i = 0; i < num_tokens; i++) {
                    free(tokens[i]); 
                }
                free(tokens); 
                tokens = NULL; 
                continue;  
            }
            // List bg processes if the second token is "list"
            if (strcmp(tokens[1], "list") == 0) {
                check_background_processes(); // check finished background processes before listing
                list_background_processes(); // List background processes
                free(prompt);
                free(line);
                for (int i = 0; i < num_tokens; i++) {
                    free(tokens[i]); 
                    tokens[i] = NULL; 
                }
                free(tokens); 
                tokens = NULL;
                prompt = NULL; 
                line = NULL; 
                continue; // Skip to the next iteration
            }
            // create new bg process if the second token is not "list"
            add_background_process(tokens + 1); // Pass the tokens excluding "bg"
            free(prompt);
            free(line);
            for (int i = 0; i < num_tokens; i++) {
                free(tokens[i]); 
            }
            free(tokens); 
            tokens = NULL; 
            prompt = NULL; 
            line = NULL;
            continue; // Skip to the next iteration
        }
        // child process 
        if ((pid = fork()) == 0) {
            execvp(tokens[0], tokens);
            perror("execvp failed in child process");
            // free(prompt);
            // free(line);
            // for (int i = 0; i < num_tokens; i++) {
            //     free(tokens[i]); 
            //     tokens[i] = NULL; 
            // }
            // free(tokens); // Free the tokens array
            // tokens = NULL; // Reset tokens to NULL to avoid dangling pointer    
            exit(1); // Exit child process if execvp fails
        }
        waitpid(pid, NULL, 0);

        // Check running background processes
        check_background_processes();

        // Free allocated memory
        free(prompt);
        free(line);
        for (int i = 0; i < num_tokens; i++) {
            free(tokens[i]); 
            tokens[i] = NULL; 
        }
        free(tokens); // Free the tokens array
        // Reset pointers to NULL to avoid dangling pointers
        tokens = NULL; 
        prompt = NULL; 
        line = NULL; 
        
    }
}
void list_background_processes() {
    if (bg_count == 0) {
        printf("No background jobs running.\n");
        return;
    }
    // Print the list of background processes
    for (int i = 0; i < bg_count; i++) {
        printf("%d: %s\n", bg_pids[i], arguments[i]);
    }
    printf("Total background jobs: %d\n", bg_count);
}
void check_background_processes() {
    for(int i = 0; i < bg_count; i++) {
        int pid = bg_pids[i];
        if (waitpid(pid, NULL, WNOHANG) > 0) {
            // Process has finished
            printf("%d %s has terminated.\n", pid, arguments[i]);
            free(arguments[i]);
            arguments[i] = NULL;  
            // Shift remaining processes down
            for (int j = i; j < bg_count; j++) {
                bg_pids[j] = bg_pids[j + 1]; // Shift PIDs down
                arguments[j] = arguments[j + 1]; // Shift commands down
            }
            bg_count--; // Decrease the count of background processes
            i--; // Adjust index to account for the shift
            bg_pids[bg_count] = 0; // Clear the last PID
            arguments[bg_count] = NULL; // Null-terminate the array of commands
        }
    }
    // If no background processes are left, free the memory
    if (bg_count == 0) {
        free(arguments); 
        arguments = NULL; 
        free(bg_pids); //
        bg_pids = NULL;
    }

}
void add_background_process(char **tokens) {
    int pid;
    char *command = tokens[0]; 
    char *arg = tokens[1]; 
    char *cwd = getcwd(NULL, 0); 
    if (bg_count == 0){
        // Allocate memory for the first time if no background processes exist
        arguments = malloc(bg_cap * sizeof(char *));
        bg_pids = calloc(bg_cap, sizeof(int));
        if (arguments == NULL || bg_pids == NULL) {
            perror("Malloc for background processes failed");
            exit(1);
        }
    }
    if (bg_count >= bg_cap) {
        // Resize the arrays if capacity is reached
        bg_cap *= 2;
        char **new_arguments = realloc(arguments, bg_cap * sizeof(char *));
        int *new_pids = realloc(bg_pids, bg_cap * sizeof(int));
        if (new_arguments == NULL || new_pids == NULL) {
            perror("Realloc for background processes failed");
            exit(1);
        }
        arguments = new_arguments; 
        bg_pids = new_pids; // Update the pointer to the new allocoted memory
    }
    if ((pid = fork()) == 0) {
        execvp(tokens[0], tokens);
        perror("execvp failed in child process");
        exit(1); // Exit child process if execvp fails    
    }
    bg_pids[bg_count] = pid; // Store the PID of the background process
    // Allocate memory for the command string
    arguments[bg_count] = malloc(strlen(cwd) + strlen(command) + strlen(arg) + 3); 
    if (arguments[bg_count] == NULL) {
        perror("malloc failed for background command");
        exit(1);
    }
    // Format the command string with current working directory and arguments
    snprintf(arguments[bg_count], strlen(cwd) + strlen(command) + strlen(arg) + 3, "%s/%s %s", cwd, command, arg);  
    bg_count++;

    //printf("Background process started with PID: %d\n", pid);
    arguments[bg_count] = NULL; // Null-terminate the array of arguments
    free(cwd); 
}

void change_directory(char **path) {
    char *home = getenv("HOME"); // Get the HOME environment variable
    // replace '~' with the home directory
    if ((*path[0]=='~')){
        char *new_path = malloc(strlen(home) + strlen(*path));
        if (new_path == NULL) {
            perror("malloc failed");
            return;
        }
        sprintf(new_path, "%s%s", home, *path + 1); // Skip the '~' character
        printf("Changing directory to: %s\n", new_path);
        *path = new_path; // Update caller's path to the new path
    }
    // change directory to the path
    if (chdir(*path) != 0) {
        perror("chdir failed");
    }
    free(*path);
}

int create_prompt(char **prompt_main) {
    char hostname[256];
    char *prompt, *username, *cwd;
    // get username
    username = getlogin();
    if (username == NULL) {
        perror("getlogin failed");
        exit(1);
    }
    // get hostname
    gethostname(hostname, sizeof(hostname));
    if (hostname == NULL) {
        perror("gethostname failed");
        exit(1);
    }
    // get current working directory
    cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("getcwd failed");
        exit(1);    
    }
    // creating prompt
    prompt = malloc(strlen(username) + strlen(hostname) + strlen(cwd) + 7); // 7bytes for "@ : > and null terminator"
    if (prompt == NULL) {
        perror("Create prompt failed");
        free(cwd);
        *prompt_main = NULL; // Set to NULL to avoid dangling pointer
        exit(1);
    }
    // formating the prompt
    sprintf(prompt, "%s@%s: %s > ", username, hostname, cwd);
    free(cwd); 
    *prompt_main = prompt; // Update the pointer to the prompt
    return 0;
}

int tokenize(char *line, char ***tokens_main){ // inpired by tokenize function from Assignment 1
    char *t;
    int num_tokens = 0;
    int tokens_capacity = 10;

    // Allocate initial memory for tokens
    char **tokens = malloc(tokens_capacity * sizeof(char *));
    if (tokens == NULL) {
        perror("malloc failed for tokens");
        exit(1);
    }
   
    t = strtok(line, " ");
    while(t != NULL){
        if (num_tokens >= tokens_capacity) {
            tokens_capacity *= 2; // Double the capacity
            char **buffer = realloc(tokens, tokens_capacity * sizeof(char *));
            if (buffer == NULL) {
                perror("realloc failed");
                free(tokens);
                exit(1);
            }
            tokens = buffer; // Update tokens pointer to the reallocated memory
        }
        tokens[num_tokens] = strdup(t); 
        t = strtok(NULL, " ");
        num_tokens++;

    }
    tokens[num_tokens] = NULL; // Null-terminate the array of tokens
    *tokens_main = tokens; // Update the pointer to the tokens array
    return num_tokens; 
}

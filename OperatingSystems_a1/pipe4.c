#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define line_size 256
#define MAX_COMMANDS 4
#define MAX_TOKENS 8


int tokenize(char *line, char *tokens[MAX_TOKENS]);
int main(){
    char *commands[MAX_COMMANDS][MAX_TOKENS];
    char *tokens[MAX_TOKENS];
    char *t;
    int num_tokens;
    int num_commands = 0;
    char line[line_size];
    int bytes_read;
    char lines[MAX_COMMANDS][line_size];
    int pid1, pid2, pid3, pid4;
    int pipe1[2], pipe2[2], pipe3[2];

  // Read lines from stdin until empty line or 4 lines
    for(int i = 0; i < MAX_COMMANDS; i++){
        bytes_read = read(STDIN_FILENO, lines[i], sizeof(line)-1);
        if (bytes_read <= 0){
            //perror("Error reading from stdin");
            return 1;
        }
        // blank line 
        else if(lines[i][0] == '\n' || lines[i][0] == '\0'){
            break;
        }
        // Replace newline character with null terminator
        if (lines[i][bytes_read - 1] == '\n') {
            lines[i][bytes_read - 1] = '\0'; 
        }
        else{
            lines[i][bytes_read] = '\0'; 

        }
        num_commands++;
    }
    switch(num_commands){
        case 0:
            //printf("No commands entered.\n");
            exit(1);
            break;
        case 1:
            if((pid1 = fork()) == 0){
                num_tokens = tokenize(lines[0], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execvp failed in first child");
            }
            waitpid(pid1, NULL, 0);

            break;
        case 2:
            // Create a pipe for the two commands
            if (pipe(pipe1) == -1) {
                // perror("pipe1");
                exit(1);
            }
            // First child process
            if((pid1 = fork()) == 0){
                close(pipe1[0]); 
                dup2(pipe1[1], STDOUT_FILENO); 
                close(pipe1[1]); 
                num_tokens = tokenize(lines[0], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execvp failed");
                exit(2);
            }
            // Second child process
            if((pid2 = fork()) == 0){
                close(pipe1[1]); 
                dup2(pipe1[0], STDIN_FILENO); 
                close(pipe1[0]); 
                num_tokens = tokenize(lines[1], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execlp2 failed");
                exit(2);
            }
            // close the pipe ends in the parent process
            close(pipe1[0]); 
            close(pipe1[1]); 
            // Wait for both child processes to finish
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            break;
        case 3:
            // create two pipes for the three commands
            if (pipe(pipe1) == -1) {
                // perror("pipe1");
                exit(1);
            }
            if (pipe(pipe2) == -1) {
                // perror("pipe2");
                exit(1);
            }
            // First Child Process
            if((pid1 = fork()) == 0){
                close(pipe1[0]);
                dup2(pipe1[1], STDOUT_FILENO);
                close(pipe1[1]);
                close(pipe2[0]); close(pipe2[1]);
                num_tokens = tokenize(lines[0], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execvp failed in first child");
                exit(2);
            }
            // Second Child Process
            if((pid2 = fork()) == 0){
                close(pipe1[1]);
                close(pipe2[0]);
                dup2(pipe1[0], STDIN_FILENO);
                dup2(pipe2[1], STDOUT_FILENO);
                close(pipe1[0]);
                close(pipe2[1]);
                num_tokens = tokenize(lines[1], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execvp failed in second child");
                exit(2);
            }
            // Third Child Processs
            if((pid3 = fork()) == 0){
                close(pipe1[0]);
                close(pipe1[1]);
                close(pipe2[1]);
                dup2(pipe2[0], STDIN_FILENO);
                close(pipe2[0]);
                num_tokens = tokenize(lines[2], tokens);
                execve(tokens[0], tokens, NULL);
                // perror("execvp failed in third child");
                exit(2);
            }
            // close the pipe ends in the parent process
            close(pipe1[0]); close(pipe1[1]);
            close(pipe2[0]); close(pipe2[1]);
            // Wait for all child processes to finish
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            waitpid(pid3, NULL, 0);
            break;
        case 4:
            // create three pipes for the four commands
            if (pipe(pipe1) == -1) {
                //perror("pipe1");
                exit(1);
            }
            if (pipe(pipe2) == -1) {
                //perror("pipe2");
                exit(1);
            }
            if (pipe(pipe3) == -1) {
                //perror("pipe3");
                exit(1);
            }
            // First Child Process
            if((pid1 = fork()) == 0){
                close(pipe3[0]); close(pipe3[1]);
                close(pipe2[0]); close(pipe2[1]);
                close(pipe1[0]);
                dup2(pipe1[1], STDOUT_FILENO);
                close(pipe1[1]);
                num_tokens = tokenize(lines[0], tokens);
                execve(tokens[0], tokens, NULL);
                exit(2);
                //perror("execvp failed in first child");
            }
            // Second Child Process
            if((pid2 = fork()) == 0){
                close(pipe3[0]); close(pipe3[1]);
                close(pipe1[1]);
                close(pipe2[0]);
                dup2(pipe1[0], STDIN_FILENO);
                dup2(pipe2[1], STDOUT_FILENO);
                close(pipe1[0]);
                close(pipe2[1]);
                num_tokens = tokenize(lines[1], tokens);
                execve(tokens[0], tokens, NULL);
                exit(2);
                //perror("execvp failed in second child");
            }
            // Third Child Process
            if((pid3 = fork()) == 0){
                close (pipe1[0]); close(pipe1[1]);
                close(pipe2[1]);
                close(pipe3[0]);
                dup2(pipe2[0], STDIN_FILENO);
                dup2(pipe3[1], STDOUT_FILENO);
                close(pipe3[1]);
                close(pipe2[0]);
                num_tokens = tokenize(lines[2], tokens);
                execve(tokens[0], tokens, NULL);
                exit(2);
                //perror("execvp failed in third child");
            }
            // Fourth Child Process
            if((pid4 = fork()) == 0){
                close(pipe1[0]); close(pipe1[1]);
                close(pipe2[0]); close(pipe2[1]);
                close(pipe3[1]);
                dup2(pipe3[0], STDIN_FILENO);
                close(pipe3[0]);
                num_tokens = tokenize(lines[3], tokens);
                execve(tokens[0], tokens, NULL);
                exit(2);
                //perror("execvp failed in fourth child");
            }
            // close the pipe ends in the parent process
            close(pipe1[0]); close(pipe1[1]);
            close(pipe2[0]); close(pipe2[1]);
            close(pipe3[0]); close(pipe3[1]);
            // Wait for all child processes to finish
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            waitpid(pid3, NULL, 0);
            waitpid(pid4, NULL, 0);
            break;
        default:
            exit(1);
            break;
    }

}
int tokenize(char *line, char *tokens[MAX_TOKENS]){
    char *t;
    int num_tokens = 0;

    t = strtok(line, " ");
    while(t != NULL){
        tokens[num_tokens] = t;
        num_tokens++;
        t = strtok(NULL, " ");
    }
    tokens[num_tokens] = NULL; // Null-terminate the array of tokens
    return num_tokens;
}
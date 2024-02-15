/*
    File: mysh.c
    Name: Rachel Nguyen
    Desc: A simple shell (mysh) that accept lines of text as input and execute 
    programs in response
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 50 // including NULL-terminated
#define DELIM " \t\n"
#define EXIT "exit" // built-in commands
#define HISTORY "history" // built-in commands
#define MAX_HIST 50

char **parse_command(char *command_line);
int check_blank_input(char *command_line);
int is_built_in(char *args);
void print_history(int hist_size);

int main() {
    // initialize and manage history
    int hist_size = MAX_HIST;
    char *hist_size_str = getenv("HISTSIZE");
    if (hist_size_str != NULL) {
        hist_size = atoi(hist_size_str);
    }
    using_history();
    stifle_history(hist_size);

    // continuosly prompt user to enter command
    while (1) {
        // print prompt
        char *command_line = readline("$ ");

        if (check_blank_input(command_line)) {
            free(command_line);
            continue;
        }

        add_history(command_line);

        // parse command line into argument
        char **args = parse_command(command_line);
        free(command_line);
        if (args == NULL) {
            printf("Error parsing command.\n");
            continue;
        }

        // check whether argument is built in command
        if (is_built_in(args[0])) {
            // exit
            if (strcmp(EXIT, args[0]) == 0) {
                printf("Goodbye!\n");
                for (int i = 0; args[i] != NULL; i++) {
                    free(args[i]);
                }
                free(args);
                break;
            }
            // history
            else if (strcmp(HISTORY, args[0]) == 0) {
                print_history(hist_size);
                // Free allocated memory
                for (int i = 0; args[i] != NULL; i++) {
                    free(args[i]);
                }
                free(args);
            } 
        } else {
            // execute command
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                for (int i = 0; args[i] != NULL; i++) {
                    free(args[i]);
                }
                free(args);
                exit(EXIT_FAILURE);
            } else if (pid == 0) {
                // child process executes
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                    for (int i = 0; args[i] != NULL; i++) {
                        free(args[i]);
                    }
                    free(args);
                    exit(EXIT_FAILURE);
                } 
            } else { 
                // parent process waits
                int status;
                waitpid(pid, &status, 0);
            }

            for (int i = 0; args[i] != NULL; i++) {
                free(args[i]);
            }
            free(args);
        }
    }
    return 0;
}

/*
    Parse the user input into the argument, return NULL if failed
*/
char **parse_command(char *command_line) {
    char **args = malloc(MAX_ARGS * sizeof(char *)); // a list of args to be returned
    if (args == NULL) {
        perror("malloc");
        return NULL;
    }

    // tokenize the input
    char *token = strtok(command_line, DELIM);
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = strdup(token); 
        if (args[i] == NULL) {
            perror("strdup");

            for (int j = 0; j < i; j++) {
                free(args[j]);
            }
            free(args);

            return NULL;
        }
        token = strtok(NULL, DELIM);
        i++;
    }    

    if (i == MAX_ARGS - 1 && token != NULL) {
        printf("Error: Too many arguments\n");
        for (int j = 0; j < i; j++) {
            free(args[j]);
        }
        free(args);
        return NULL;
    }

    args[i] = NULL; 
    return args;
}

/*
    Check if user enter nothing or a list of white spaces
*/
int check_blank_input(char *command_line) {
    if (command_line == NULL) {
        return 1;
    } 

    while (*command_line != '\0') {
        if (!isspace(*command_line)) {
            return 0;
        }
        command_line++;
    }
    return 1;
}

/*
    Check if the input command is a built-in argument
*/
int is_built_in(char *args) {
    char *built_in_commands[] = {EXIT, HISTORY, NULL};
    for (int i = 0; built_in_commands[i] != NULL; i++) {
        if (strcmp(args, built_in_commands[i]) == 0) {
            return 1; 
        }
    }
    return 0;
}

/*
    Print history
*/
void print_history(int hist_size) {
    HIST_ENTRY **hist_list = history_list();
    if (hist_list == NULL) {
        perror("history_list");
        return;
    }

    for (int i = 0; i < hist_size && hist_list[i] != NULL; i++) {
        int hist_num = i+1;
        printf("%d: %s\n", hist_num, hist_list[i]->line);
    }
}
/*
    File: mysh.c
    Name: Rachel Nguyen, Julia Rieger
    Desc: A simple shell (mysh) that accept lines of text as input and execute 
    programs in response
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <stdbool.h>

#define MAX_ARGS 51 // max number of arguments, including NULL
#define DELIM " \t\n&" // delimiters to tokenize user input command
#define EXIT "exit" // built-in command: exit

char **parse_command(char *command_line, bool *background);
void execute_command(char **args, bool *background);
int check_blank_input(char *command_line);
int is_built_in(char *args);
void free_args(char **args);

int main() {
    // continuosly prompt user to enter command
    while (true) {
        // print prompt
        char *command_line = readline("$ ");

        if (check_blank_input(command_line)) {
            free(command_line);
            continue;
        }

        // parse command line into argument
        bool background = false;
        char **args = parse_command(command_line, &background);
        free(command_line);
        if (args == NULL) {
            printf("Error parsing command.\n");
            continue;
        }

        // check whether argument is built in command, then execute in fg
        if (is_built_in(args[0])) {
            // exit
            if (strcmp(EXIT, args[0]) == 0) {
                printf("Goodbye!\n");
                free_args(args);
                break;
            }
        } else {
            // execute command
            execute_command(args, &background);
            free_args(args);
        }
    }
    return 0;
}

/**
 * @brief Parse the user input into the argument
 *
 * @param command_line Input from user
 * @return An array of args on success, NULL on error
 */
char **parse_command(char *command_line, bool *background) {
    // check if user enter only delim
    bool is_input_delim = strspn(command_line, DELIM) == strlen(command_line);
    if (is_input_delim) {
        return NULL;
    }

    char **args = malloc(MAX_ARGS * sizeof(char *)); // a list of args to be returned
    if (args == NULL) {
        perror("malloc");
        return NULL;
    }

    // check if user enter &
    size_t len = strlen(command_line);
    *background = (command_line[len - 1] == '&');

    // tokenize the input
    char *token = strtok(command_line, DELIM);
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = strdup(token); 
        if (args[i] == NULL) {
            perror("strdup");
            free_args(args);
            return NULL;
        }
        token = strtok(NULL, DELIM);
        i++;
    }    

    if (i == MAX_ARGS - 1 && token != NULL) {
        printf("Error: Too many arguments\n");
        free_args(args);
        return NULL;
    }

    args[i] = NULL; 
    return args;
}

/**
 * @brief Check whether user enters a blank input
 *
 * @param command_line Input from user
 * @return true if it is a blank input, false otherwise
 */
int check_blank_input(char *command_line) {
    if (command_line == NULL) {
        return true;
    } 

    while (*command_line != '\0') {
        if (!isspace(*command_line)) {
            return false;
        }
        command_line++;
    }

    return true;
}

/**
 * @brief Check if an argument is a built-in command
 *
 * @param args The argument (after parsed)
 * @return true if it is a built-in command, false otherwise
 */
int is_built_in(char *args) {
    char *built_in_commands[] = {EXIT, NULL};
    for (int i = 0; built_in_commands[i] != NULL; i++) {
        if (strcmp(args, built_in_commands[i]) == 0) {
            return true; 
        }
    }
    return false;
}

/**
 * @brief Free the argument (an array of strings)
 *
 * @param args The argument 
 */
void free_args(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }
    free(args);
}

/**
 * @brief Execute the given command
 *
 * @param args The arguments for the command
 * @param background Flag indicating whether the command should run in the background
 */
void execute_command(char **args, bool *background) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free_args(args);
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            free_args(args);
            exit(EXIT_FAILURE);
        } 
    } else {
        if (!*background) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            printf("Sending job to background...\n");
            // don't wait for child
            // add child to the job queue
        }
    }
}
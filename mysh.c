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

#define MAX_ARGS 5 // max number of arguments, including NULL
#define DELIM " \t\n" // delimiters to tokenize user input command
#define EXIT "exit" // built-in command: exit

typedef struct {
    char *cmdline;
    bool bg;
} Command;

char **parse_command(Command *command);
void execute_command(char **args, bool bg);
int check_blank_input(char *command_line); 
int is_built_in(char *args);
void free_args(char **args);
char *remove_space(char *str);
bool check_syntax(char* input);
char **tokenize(char *input, char *delim);
Command **parse_to_structs(char *command_list);
void free_command_list(Command **commands);

int main() {
    bool exit = false;
    // continuosly prompt user to enter command
    while (true) {
        // print prompt
        char *input = readline("$ ");

        // if blank input or has syntax error related to delim, go to the next loop
        if (check_blank_input(input) || !check_syntax(input)) {
            free(input);
            continue;
        } 

        // tokenize with ; to get a list of sequential commands to be executed
        char **command_list = tokenize(input, ";");
        if (command_list == NULL) {
            printf("Error tokenizing with ;\n");
            free(input);
            continue;
        }

        // for each sequential command, find the processes
        for (int i = 0; command_list[i] != NULL; i++) {
            Command **commands = parse_to_structs(command_list[i]);
            if (commands == NULL) {
                printf("Error tokenizing with &\n");
                free(command_list[i]);
                continue;
            }
            
            // make this a func
            // for each process, parse into args then execute
            for (int j = 0; commands[j] != NULL; j++) {
                if (check_blank_input(commands[j]->cmdline)) {
                    free(commands[j]->cmdline);
                    free(commands[j]);
                    continue;
                } else {
                    // parse into args
                    char **args = parse_command(commands[j]);
                    free(commands[j]->cmdline);
                    free(commands[j]);
                    if (args == NULL) {
                        printf("Error parsing command.\n");
                        continue;
                    }
                    // execute
                    if (is_built_in(args[0])) {
                        // exit
                        if (strcmp(EXIT, args[0]) == 0) {
                            printf("Goodbye!\n");
                            free_args(args);
                            exit = true;
                            break;
                        }
                    } else {
                        // execute command
                        execute_command(args, commands[j]->bg);
                        free_args(args);
                    }
                }
            }
            free_command_list(commands);
            if (exit) {
                break;
            }
        }

        free_args(command_list);
        free(input);

        if (exit) {
            break;
        }
    }

    return 0;
}

/**
 * @brief Parse the user input into the argument
 *
 * @param command Input from user
 * @return An array of args on success, NULL on error
 */
char **parse_command(Command *command) {
    // tokenize the input
    return tokenize(command->cmdline, DELIM);
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
void execute_command(char **args, bool background) {
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
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            printf("Sending job to background...\n");
            // don't wait for child
            // add child to the job queue
        }
    }
}

bool check_syntax(char* input) {
    // check if the input starts with a delim
    bool start_with_delim = (strchr("&;", input[0]) != NULL);
    if (start_with_delim) {
        printf("Error: Syntax error near unexpected token '&' or ';'\n");
        return false;
    }

    // check for "&&", "&;", ";&", or ";;"
    char *input_without_space = remove_space(input);
    if (input_without_space == NULL) {
        return false;
    }

    char* error_combinations[] = {"&&", "&;", ";&", ";;"};
    int num_combinations = sizeof(error_combinations) / sizeof(error_combinations[0]);

    for (int i = 0; i < num_combinations; i++) {
        if (strstr(input_without_space, error_combinations[i]) != NULL) {
            printf("Error: Syntax error near unexpected token '&' or ';'\n");
            free(input_without_space);
            return false; 
        }
    }
    free(input_without_space);
    return true; 
}

char *remove_space(char *str) {
    int len = strlen(str);
    char* result = (char *) malloc((len + 1) * sizeof(char)); 
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < len; i++) {
        if (!isspace(str[i])) { // check if the character is not a whitespace
            result[j++] = str[i];
        }
    }
    result[j] = '\0'; // null-terminate 

    return result;
}

char **tokenize(char *input, char *delim) {
    char **tokens = (char **)malloc(MAX_ARGS * sizeof(char *));
    if (tokens == NULL) {
        perror("malloc");
        return NULL;
    }

    char *input_copy = strdup(input); // preserve the input
    if (input_copy == NULL) {
        perror("strdup");
        return NULL;
    }

    int token_count = 0;
    char *token = strtok(input_copy, delim);
    while (token != NULL && token_count < MAX_ARGS - 1) {
        tokens[token_count] = strdup(token);
        if (tokens[token_count] == NULL) {
            perror("strdup");
            free_args(tokens);
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, delim);
        token_count++;
    }

    if (token_count == MAX_ARGS - 1 && token != NULL) {
        printf("Error: Too many arguments\n");
        free_args(tokens);
        return NULL;
    }

    tokens[token_count] = NULL; // null-terminate the array of tokens
    free(input_copy); 
    return tokens;
}

Command **parse_to_structs(char *command_list) {
    // check if the command at the end of command_list has & in it
    // if it does, the last token is also a bg process
    bool flag = false;
    char *str = remove_space(command_list);
    size_t len = strlen(str);
    if (str[len - 1] == '&') {
        flag = true;
    } 
    free(str);

    // tokenize by '&' to find the bg processes and storing them in Command structs
    char **tokens = tokenize(command_list, "&");
    if (tokens == NULL) {
        printf("Error tokenizing with &\n");
        return NULL;
    }

    int count = 0; // the number of bg processes
    for (int i = 0; tokens[i] != NULL; i++) {
        count++;
    }

    Command **commands = (Command **) malloc(((count + 1) * sizeof(Command))); // null terminated
    if (commands == NULL) {
        perror("Memory allocation failed");
        free_args(tokens);
        return NULL;
    }

    // populate the list of commands
    for (int i = 0; i < count; i++) {
        commands[i] = malloc(sizeof(Command));
        if (commands[i] == NULL) {
            perror("Memory allocation failed");
            free_command_list(commands); 
            free_args(tokens);
            return NULL;
        }

        if (i == count - 1) {
            commands[i]->cmdline = strdup(tokens[i]);
            commands[i]->bg = flag; 
        } else {
            commands[i]->cmdline = strdup(tokens[i]);
            commands[i]->bg = true; 
        }
    }
    commands[count] = NULL; // null-terminate the array

    // print the list of commands
    for (int i = 0; tokens[i] != NULL; i++) {
        printf("Debug: Command %d: %s (Background: %s)\n", i + 1, commands[i]->cmdline, commands[i]->bg ? "true" : "false");
    }

    // free memory
    free_args(tokens);

    return commands;
}

void free_command_list(Command **commands) {
    // Free each command list and its elements
    for (int i = 0; commands[i] != NULL; i++) {
        free(commands[i]->cmdline);
        free(commands[i]);
    }
    // Free the array of command lists
    free(commands);
}

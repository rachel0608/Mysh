//mysh.h

#ifndef MYSH_H
#define MYSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include "JobLL.h"

#define MAX_ARGS 5 // max number of arguments, including NULL
#define DELIM " \t\n" // delimiters to tokenize user input command
#define EXIT "exit" // built-in command: exit
#define STATUS_TERMINATE    2
#define STATUS_SUSPEND  1
#define STATUS_RESUME   0
#define TRUE 1
#define FALSE 0

typedef struct {
    char *cmdline;
    bool bg;
} Command;

//global vars
volatile sig_atomic_t sigchld_received = 0; // flag for sig_chld_handler to update
volatile sig_atomic_t sigtstp_received = 0; // flag for sig_tstp_handler to update
volatile pid_t pid_to_update; //pid of Job to update
pid_t foreground_pid = -1;  // Initialize to an invalid PID, set in execute_command

//function prototypes
char **parse_command(Command *command); 
void execute_command(char **args, bool bg);
int check_blank_input(char *command_line); 
int is_built_in(char *args);
char *remove_space(char *str);
bool check_syntax(char* input);
char **tokenize(char *input, char *delim);
Command **parse_to_structs(char *command_list);
void free_args(char **args);
void free_command_list(Command **commands);
void update_joblist(pid_t pid_to_update, int status_to_update); //update JOBLL based on what signals were caught: finds job in LL with pid "pid_to_update", sets status to "status_to_update"
void block_sigchld(int block); //function to block or unblock SIGCHLD- signal masking
void sig_chld_handler(int sig, siginfo_t *info, void *ucontext); //handles SIGCHLD
void sig_tstp_handler(int signo); //handles SIGTSTP
void setup_sighandlers(); //sets up signal handlers with sigaction()

#endif

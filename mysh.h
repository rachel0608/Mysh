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
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include <errno.h>
#include "JobLL.h"

#define MAX_ARGS 5 // max number of arguments, including NULL
#define DELIM " \t\n" // delimiters to tokenize user input command
#define EXIT "exit" // built-in command: exit
#define JOBS "jobs" //built in command: jobs
#define KILL "kill" // built-in command: kill
#define FG "fg" // built-in command: fg
#define BG "bg" // built-in command: bg
#define STATUS_RESUME_FG    3
#define STATUS_TERMINATE    2
#define STATUS_SUSPEND  1
#define STATUS_RESUME   0
#define TRUE 1
#define FALSE 0

typedef struct {
    char *cmdline;
    bool bg;
} Command;

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

void update_joblist(pid_t pid_to_update, int status_to_update); //update JOBLL based on what signals were caught: finds job in LL with pid "pid_to_update", sets status to "status_to_update"
void block_sigchld(int block); //function to block or unblock SIGCHLD- signal masking
void setup_sighandlers(); //sets up signal handlers with sigaction()
void setup_child_sighandlers(); //set up Job's signal handlers with sigaction
void set_default_signal_mask(); //sets signal masks back to default for children at the shell
void save_terminal_settings(); //save shell's term settings
void restore_terminal_settings(); //restore shell's term settings
void save_child_terminal_settings(Job *child); //save child's term settings
void restore_child_terminal_settings(Job *child); //restore child's term settings

int count_strings(char **arr); // count argc
int extract_jid(const char *str); // extract number from [%#]
void bg(int argc, char **args);
void fg(int argc, char **args);
void bring_job_to_fg(Job *j);
void bring_job_to_bg(Job *j);
void kill_job(char **args);

//these need implementation help
void sig_chld_handler(int sig, siginfo_t *info, void *ucontext); //for shell to handle SIGCHLD
void sig_tstp_handler(int sig, siginfo_t *info, void *ucontext); //for child to handle SIGTSTP
void sig_cont_handler(int sig, siginfo_t *info, void *ucontext); //for child to handle SIGCONT
void handle_child(Job *child); //called from main loop, updates joblist accordingly and passes struct info
void handle_tstp(Job *child); //called from main loop, suspends job - updates joblist

#endif

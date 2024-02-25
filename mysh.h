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
#define JOBS "jobs" // built-in command: jobs
#define KILL "kill" // built-in command: kill
#define FG "fg" // built-in command: fg
#define BG "bg" // built-in command: bg
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
void execute_command(char **args, bool bg, JobLL *job_list);
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
void sig_chld_handler(int sig, siginfo_t *info, void *ucontext); //for shell to handle SIGCHLD
void sig_tstp_handler(int sig, siginfo_t *info, void *ucontext); //for child to handle SIGTSTP
void sig_cont_handler(int sig, siginfo_t *info, void *ucontext); //for child to handle SIGCONT
void setup_sighandlers(); //sets up signal handlers with sigaction()
int count_strings(char **arr); // count argc
int extract_jid(const char *str); // extract number from [%#]
void bg(int argc, char **args);
void fg(int argc, char **args);
void bring_job_to_fg(Job *j);
void bring_job_to_bg(Job *j);
void kill_job(char **args);

void setup_child_sighandlers();
void set_default_signal_mask();
void save_terminal_settings();
void restore_terminal_settings();
void save_child_terminal_settings(Job *child);
void restore_child_terminal_settings(Job *child);
void handle_child(pid_t job_pid);

#endif
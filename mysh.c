/*
    File: mysh.c
    Name: Rachel Nguyen, Julia Rieger
    Desc: A simple shell (mysh) that accept lines of text as input and execute 
    programs in response
*/

#include <signal.h>
#include "mysh.h"

//global vars
volatile pid_t pid_to_update; //pid of Job to update
volatile int status_of_child; //pid of Job to update
volatile int exit_status; //exit status of child if SIGCHLD -> CLD_EXITED 
pid_t foreground_pid = -1;  // Initialize to an invalid PID, set in execute_command
struct termios saved_terminal_settings; // Global variable to store the original terminal settings
JobLL *job_list; //global job list of all jobs

int main() {
    
    setup_sighandlers();
    bool exit = false;
    job_list = new_list();

    while (true) {
        //iter through joblist, handle jobs with flags on
            //block signals?
            //unpack struct
            //if a job's flag is on call handle_child() or handle_tstp()

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
    pid_t shell_pid = getpid(); // Save the shell's PID
    save_terminal_settings(); //Save the original terminal settings
    pid_t pid = fork();
    
    if (pid < 0) { //if error
        perror("fork");
        free_args(args);
        exit(EXIT_FAILURE);

    } else if (pid == 0) { //in child

        pid_t child_pid = getpid(); // get child's pid
        if (setpgid(child_pid, child_pid) == -1) {  // Assign the child process to a new process group with its own PID
            perror("setpgid");
            exit(EXIT_FAILURE);
        }
        setup_child_sighandlers(); //install child sig handlers
        set_default_signal_mask(); //set signal masks back to default before replacing child
        if (execvp(args[0], args) == -1) { //execute/replace child
            perror("execvp");
            free_args(args);
            exit(EXIT_FAILURE);
        } 

    } else { //in parent (shell)
        pid_t wpid;
        int status;
        setpgid(pid, pid);  // Set the child process to a new process group
        Job *child = new_job(pid, args[0]); //create Job

        if (!background) { //if job is foregrounded

            foreground_pid = pid; //set foreground pid global to be used in case of ctrl z
            if (tcsetpgrp(STDIN_FILENO, pid) == -1) { //give terminal to child
                perror("tcsetpgrp");
                exit(EXIT_FAILURE);
            }
            wpid = waitpid(pid, &status, 0); //shell waits for child process
            //if child was interrupted, sig handler will handle
            //else child is done and no action necessary from shell
            if (tcsetpgrp(STDIN_FILENO, shell_pid) == -1) { //give fg back to shell
                perror("tcsetpgrp");
                exit(EXIT_FAILURE);
            }
            foreground_pid = shell_pid; //update foreground_pid to shell
            restore_terminal_settings(); //restore original shell settings
        
        } else { //if job is backgrounded

            printf("Sending job to background...\n");
            add_job(job_list, child); //add child to LL
            wpid = waitpid(pid, &status, WUNTRACED);
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

void update_joblist(pid_t pid, int status_to_update) {
    if ((find_job(job_list, pid) < 0) || pid == foreground_pid) { //job to update is in the foreground
        if (status_to_update == STATUS_TERMINATE) {  
            //print to terminal and exit status global

        } else if (status_to_update == STATUS_SUSPEND) {
            //update job status
            //add to list
            //save pid term settings
            //give term to shell
            //restore shell term settings
            //print jobs
        }
    }
    else { //job to update is in the joblist
        if (status_to_update == STATUS_TERMINATE) {
            //update job status
            get_nth_job(find_job(job_list, pid))->status = STATUS_TERMINATE;
            //print to terminal exit status global
            //remove from list
        } else if (status_to_update == STATUS_SUSPEND) {
            //update job status
            //print jobs
        } else if (status_to_update == STATUS_RESUME) {
            kill(pid, SIGCONT);
            //update job status
            //print jobs
        } else if (status_to_update == STATUS_RESUME_FG) {
            //call fg on the job: give job term (below), update status, and resume execution
            //print jobs

            //save shell term settings
            //give term to pid
            restore_child_terminal_settings(get_nth_job(find_job(job_list, pid))); //restore pid term settings
        }
    }
	//find job in list
		//if WIFEXITED-> status = STATUS_TERMINATE, update job status, print to terminal (also print exit status global), and remove from list
        	//if WIFSIGNALED child process terminated by signal -> killed -> treat same as WIFEXITED?
        //if WIFSTOPPED-> status=STATUS_SUSPEND, (a fg running job was stopped/blocked), update job status, add to list, print jobs
		//if none (default) -> status=STATUS_RESUME (aka resume suspended job) kill -CONT <pid>, update job status, print jobs
            //remember restore_child_terminal_settings when resuming in fg


    hangle_flag_true(get_nth_job(find_job(job_list, pid)), FALSE) //aka reset flag

}

void block_sigchld(int block) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGCHLD);

    if (block) {
        sigprocmask(SIG_BLOCK, &sigset, NULL);
    } else {
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
    }
}

//set Job's flag-> make a struct to contain flag, status, and exit status?
void sig_chld_handler(int sig, siginfo_t *info, void *ucontext) {
    block_sigchld(TRUE); //block SIGCHLD
    get_nth_job(find_job(job_list, info->si_pid))->sigchld_flag = info->si_code; //set flag to status of child
        //handle_flag_true(get_nth_job(job_list, (job_list, job_pid)), TRUE); //set job's flag

    /* pid_to_update = info->si_pid;	//store pid and status and exit status in globals (critical region)
    status_of_child = info->si_code;
    exit_status = info->si_status; */
    block_sigchld(FALSE); //unblock SIGCHLD
}

//to be called from main loop to do necessary work on a SIGCHLDed job
void handle_child(Job *child) {
    //unpack struct/recieve struct from shell containing info: child status and exit status
    if (status_of_child == CLD_EXITED || status_of_child == CLD_KILLED)
        update_joblist(child->pid, STATUS_TERMINATE); //terminate job with pid_to_update	
    else if (status_of_child == CLD_STOPPED)
	    update_joblist(child->pid, STATUS_SUSPEND); //suspend job with pid_to_update	
}

//set Job's flag->interrupted
void sig_tstp_handler(int sig, siginfo_t *info, void *ucontext) {
    block_sigchld(TRUE); //block SIGCHLD
    handle_tstpflag_true(get_nth_job(job_list, (job_list, info->si_pid)), TRUE); //set job's flag
    block_sigchld(FALSE); //unblock SIGCHLD
} 

//to be called from main loop to do necessary work on an interrupted/stopped job
void handle_tstp(Job *child) {
    save_child_terminal_settings(child); //save terminal settings for Job if child was suspended
    update_joblist(child->pid, STATUS_SUSPEND); //suspend job with pid_to_update 
}

void sig_cont_handler(int sig, siginfo_t *info, void *ucontext) {
    //handle SIGCONT from parent process attempting to resume child process that is suspended
}

void setup_sighandlers() {
    //ignore interrupt signals
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    struct sigaction sa_chld;
    //set up signal handler for SIGCHLD
    sa_chld.sa_sigaction = sig_chld_handler;     
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_SIGINFO; 
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) { 
	    perror("Error setting up sigchld handler"); 
	    exit(EXIT_FAILURE); 
    }
}

void setup_child_sighandlers() {
    struct sigaction sa_tstp;
    //set up signal handler for SIGCHLD
    sa_tstp.sa_sigaction = sig_tstp_handler;     
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART | SA_SIGINFO; 
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) { 
	    perror("Error setting up sigtstp handler"); 
	    exit(EXIT_FAILURE); 
    }
}

void set_default_signal_mask() {
    sigset_t default_mask;
    
    // Initialize a signal set representing the default signal mask
    if (sigemptyset(&default_mask) == -1 || sigfillset(&default_mask) == -1) {
        perror("sigemptyset/sigfillset");
        exit(EXIT_FAILURE);
    }

    // Set the default signal mask
    if (sigprocmask(SIG_SETMASK, &default_mask, NULL) == -1) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
}

void save_terminal_settings() {
    // Save the current terminal settings
    if (tcgetattr(STDIN_FILENO, &saved_terminal_settings) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
}

void restore_terminal_settings() {
    // Restore the original terminal settings
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &saved_terminal_settings) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void save_child_terminal_settings(Job *child) {
    // Save the current terminal settings for the child
    if (tcgetattr(STDIN_FILENO, child->setting) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
}

void restore_child_terminal_settings(Job *child) {
    // Restore the original terminal settings for the child
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, child->setting) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

/*
    File: mysh.c
    Name: Rachel Nguyen, Julia Rieger
    Desc: A simple shell (mysh) that accept lines of text as input and execute 
    programs in response
*/

#include "mysh.h"

//global vars
pid_t foreground_pid = -1;  // Initialize to an invalid PID, set in execute_command
char **foreground_command; //init every fg command here
struct termios saved_terminal_settings; // Global variable to store the shell's terminal settings
JobLL *job_list; //global job list of all jobs
pid_t shell_pid; //saves the pid of the shell

int main() {
    
    setup_sighandlers();
    bool exit = false;
    job_list = new_list();
    shell_pid = getpid();

    while (true) {

        //iter through joblist + handle jobs with flags on
        block_sigchld(TRUE); //block SIGCHLD
        printf("about to iter\n");
        struct Node *current = job_list->head;
        for (int i = 0; i < size(job_list); i++) {
            if (current->data->exit_info.chld_flag == 1) { //handle chld flag
                handle_child(current->data); //handle job
                current->data->exit_info.chld_flag = 0; //reset flag
            }
            if (current->data->tstp_flag == 1) { //handle tstp flag
                handle_tstp(current->data); //handle job
                current->data->tstp_flag = 0; //reset flag
            }
            current = current->next;
        }
        block_sigchld(FALSE); //unblock SIGCHLD

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
                        } else if (strcmp(JOBS, args[0]) == 0) {
                            printf("Test print jobs\n");
                            print_jobs(job_list);
                        } else if (strcmp(KILL, args[0]) == 0) {
                            // Send a SIGTERM to the specified job and terminate it. You will notice that this does
                            //not terminate many applications, as they register signal handlers to catch SIGTERM. 
                            // Implement also the -9 flag to send SIGKILL, which can not be caught.
                            kill_job(args);
                        } else if (strcmp(FG, args[0]) == 0) {
                            int argc = count_strings(args);
                            fg(argc, args);
                        } else if (strcmp(BG, args[0]) == 0) {
                            int argc = count_strings(args);
                            bg(argc, args);
                        }
                    } else {
                        // execute command
                        execute_command(args, commands[j]->bg);
                        free_args(args);
                        free(commands[j]);
                    }
                }
            }
            if (exit) {
                break;
            }
            //test
            //free_args(command_list);
            free(input);
            
        }
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
    char *built_in_commands[] = {EXIT, JOBS, KILL, FG, BG, NULL};
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
    save_terminal_settings(); //Save the original terminal settings
    pid_t pid = fork();
    
    if (pid < 0) { //if error
        perror("fork");
        free_args(args);
        return;

    } else if (pid == 0) { //in child

        pid_t child_pid = getpid(); // get child's pid
        if (setpgid(child_pid, child_pid) == -1) {  // Assign the child process to a new process group with its own PID
            perror("setpgid");
            return;
        }
        set_default_signal_mask(); //set signal masks back to default before replacing child
        setup_child_sighandlers(); //install child sig handlers
        
        if (execvp(args[0], args) == -1) { //execute/replace child
            perror("execvp");
            free_args(args);
            return;
        } 

    } else { //in parent (shell)
        pid_t wpid;
        int status;
        setpgid(pid, pid);  // Set the child process to a new process group

        if (!background) { //if job is foregrounded
            block_sigchld(TRUE); //block signals before accessing shared memory
            foreground_pid = pid; //set foreground pid global to be used in case of ctrl z
            foreground_command = args; //set foreground command global
            block_sigchld(FALSE); //unblock signals after accessing shared memory
            if (tcsetpgrp(STDIN_FILENO, pid) == -1) { //give terminal to child
                perror("tcsetpgrp");
                return;
            }
            wpid = waitpid(pid, &status, 0); //shell waits for child process
            //if child was interrupted, sig handler will handle
            //else child is done and no action necessary from shell
            if (tcsetpgrp(STDIN_FILENO, shell_pid) == -1) { //give fg back to shell
                perror("tcsetpgrp");
                return;
            }
            block_sigchld(TRUE); //block signals before accessing shared memory
            foreground_pid = shell_pid; //update foreground_pid to shell
            block_sigchld(FALSE); //unblock signals after accessing shared memory
            restore_terminal_settings(); //restore original shell settings
        
        } else { //if job is backgrounded
            Job *child = new_job(pid, args); //create Job
            printf("[%d] %d\n", child->jid, pid); //print: [jid] <pid>
            block_sigchld(TRUE); //block signals before accessing shared memory
            add_job(job_list, child); //add child to LL
            block_sigchld(FALSE); //unblock signals after accessing shared memory
            wpid = waitpid(pid, &status, WNOHANG | WUNTRACED);
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
            //free_args(tokens);
            return NULL;
        }
        token = strtok(NULL, delim);
        token_count++;
    }

    if (token_count == MAX_ARGS - 1 && token != NULL) {
        printf("Error: Too many arguments\n");
        //free_args(tokens);
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

void update_joblist(pid_t pid, int status_to_update) {
    block_sigchld(TRUE); //block signals before accessing shared memory            
    if ((pid == foreground_pid || find_job(job_list, pid) < 0)) { //job to update is in the foreground
        if (status_to_update == STATUS_TERMINATE) {  

            //do not print anything-do nothing?
            return;

        } else if (status_to_update == STATUS_SUSPEND) {

            printf("print in updatejobs\n");
            if (tcsetpgrp(STDIN_FILENO, shell_pid) == INVALID) { //give term back to shell
                perror("tcsetpgrp");
                return;
            } 
            restore_terminal_settings(); //restore shell term settings
            print_jobs(job_list); //print jobs
            //make job last job suspended/backgrounded?
            return;
        }
    }
    else { //job to update is in the joblist
        if (status_to_update == STATUS_TERMINATE) {

            int index = find_job(job_list, pid);
            Job *temp = get_nth_job(job_list, index);
            temp->status = STATUS_TERMINATE; //update job status
            printf("[%d] Done\t\t%s\n", temp->jid, temp->command[0]); //print to terminal exit status global: [jid]+ Done\t\t./p
            temp = NULL; //unlink pointer to avoid misuse
            free(remove_nth_job(job_list, index)); //remove from list
            

        } else if (status_to_update == STATUS_SUSPEND) {
            
            Job *temp = get_nth_job(job_list, find_job(job_list, pid));
            temp->status = STATUS_SUSPEND; //update job status
            printf("[%d] Stopped\t\t%s\n", temp->jid, temp->command[0]); //print status
            temp = NULL;
            //make temp last job suspended?

        } /*else if (status_to_update == STATUS_RESUME) {
            
            kill(-pid, SIGCONT); //send continue signal to all processes in PG
            //in sigcont handler
                //update job status
                //print jobs
                //make job last job resumed?

        } else if (status_to_update == STATUS_RESUME_FG) {
            
            //if fg is called on the job: give job the terminal (below), update status, and resume execution
            //print jobs

            //save shell term settings
            //give term to pid
            restore_child_terminal_settings(get_nth_job(job_list, find_job(job_list, pid))); //restore pid term settings
        }*/
    }

    block_sigchld(FALSE); //unblock signals after accessing shared memory
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
    if (info->si_pid == foreground_pid) {
       
    }
    else {
        block_sigchld(TRUE); //block signals before accessing shared memory
        Job *temp = get_nth_job(job_list, find_job(job_list, info->si_pid));
        temp->exit_info.chld_flag = TRUE; //turn on child's flag
        temp->exit_info.status_of_child = info->si_code; //store status of child
        temp->exit_info.exit_status = info->si_status; //store exit status of interruption
        temp = NULL; //unlink pointer to avoid misuse
        block_sigchld(FALSE); //unblock signals after accessing shared memory
    }
}

//to be called from main loop to do necessary work on a SIGCHLDed job
void handle_child(Job *child) {
    if (child->exit_info.status_of_child == CLD_EXITED || child->exit_info.status_of_child == CLD_KILLED)
        update_joblist(child->pid, STATUS_TERMINATE); //terminate child
    else if (child->exit_info.status_of_child == CLD_STOPPED)
	    //do nothing, already handled by handle_tstp()
        return;
}

//set Job's flag->interrupted
void sig_tstp_handler(int sig, siginfo_t *info, void *ucontext) {
    //block_sigchld(TRUE); //block signals before accessing shared memory

    //Job *suspended_job = new_job(info->si_pid, foreground_command); //create new job
    //suspended_job->status = STATUS_SUSPEND; //update job status
    //save_child_terminal_settings(suspended_job); //save pid term settings
    //suspended_job->tstp_flag = TRUE; //turn on interrupt flag
    //add_job(job_list, suspended_job); //add to list
    return;
    //block_sigchld(FALSE); //unblock signals after accessing shared memory
} 

//to be called from main loop to do necessary work on an interrupted/stopped job
void handle_tstp(Job *child) {
    printf("in handle_tstp()\n");
    update_joblist(child->pid, STATUS_SUSPEND); //suspend child
}

void sig_cont_handler(int sig, siginfo_t *info, void *ucontext) {
    update_joblist(getpid(), STATUS_RESUME_FG);
    //handle SIGCONT from parent process attempting to resume child process that is suspended
}

void sig_int_handler(int signo) {
    return;
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
	    return;
    }
}

void setup_child_sighandlers() {
    struct sigaction sa_tstp;
    //set up signal handler for SIGTSTP
    sa_tstp.sa_sigaction = sig_tstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART | SA_SIGINFO; 
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) { 
	    perror("Error setting up sigtstp handler"); 
	    return; 
    }

    struct sigaction sa_int;
    sa_int.sa_handler = sig_int_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART | SA_SIGINFO; 
    if (sigaction(SIGINT, &sa_int, NULL) == -1) { 
	    perror("Error setting up sigint handler"); 
	    return; 
    }
}

void set_default_signal_mask() {
    sigset_t default_mask;
    /* sigaddset(&default_mask, SIGTSTP);
    sigaddset(&default_mask, SIGTERM);
    sigaddset(&default_mask, SIGINT);
    sigaddset(&default_mask, SIGTTIN);
    sigaddset(&default_mask, SIGTTOU);
    sigaddset(&default_mask, SIGQUIT); */

    // make default mask empty
    if (sigemptyset(&default_mask) == INVALID) {
        perror("sigemptyset/sigfillset");
        return;
    }

    // mask the empty set (mask 0 signals)
    if (sigprocmask(SIG_SETMASK, &default_mask, NULL) == INVALID) {
        perror("sigprocmask");
        return;
    }
}

void save_terminal_settings() {
    // Save the current terminal settings
    block_sigchld(TRUE); //block signals before accessing shared memory
    if (tcgetattr(STDIN_FILENO, &saved_terminal_settings) == -1) {
        block_sigchld(FALSE); //unblock signals after accessing shared memory
        perror("tcgetattr");
        return;
    }
    block_sigchld(FALSE); //unblock signals after accessing shared memory
}

void restore_terminal_settings() {
    // Restore the original terminal settings
    block_sigchld(TRUE); //block signals before accessing shared memory
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &saved_terminal_settings) == -1) {
        block_sigchld(FALSE); //unblock signals after accessing shared memory
        perror("tcsetattr");
        return;
    }
    block_sigchld(FALSE); //unblock signals after accessing shared memory
}

void save_child_terminal_settings(Job *child) {
    // Save the current terminal settings for the child
    if (tcgetattr(STDIN_FILENO, child->setting) == -1) {
        perror("tcgetattr");
        return;
    }
}

void restore_child_terminal_settings(Job *child) {
    // Restore the original terminal settings for the child
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, child->setting) == -1) {
        perror("tcsetattr");
        return;
    }
}

int count_strings(char **arr) {
    int count = 0;
    while (arr[count] != NULL) {
        count++;
    }
    return count;
}

int extract_jid(const char *str) {
    // Check if the string starts with '%' followed by a number
    if (str[0] == '%' && atoi(&str[1]) != 0) {
        return atoi(&str[1]);
    } else {
        // Return -1 to indicate failure (if the string doesn't match the format)
        return -1;
    }
}

void bg(int argc, char **args) {
    Job *j;
    // if no jid is specified
    if (argc < 2) {
        j = get_last_suspended_job(job_list);
    } else { // if jid is specified
        int jid = extract_jid(args[1]);
        if (jid == -1) {
            printf("Invalid jid");
            return;
        }
        // find the job
        j = find_job_jid(job_list, jid);
    }
    if (j == NULL) {
        printf("Cannot find job with specified jid\n");
        return;
    } 
    // move the specified job to the foreground
    bring_job_to_bg(j);
} //resume most recently suspended job in bg

void fg(int argc, char **args) {
    Job *j;
    // if no jid is specified
    if (argc < 2) {
        // find the last job backgrounded
    } else { // if jid is specified
        int jid = extract_jid(args[1]);
        if (jid == -1) {
            printf("Invalid jid");
            return;
        }
        // find the job
        j = find_job_jid(job_list, jid);
    }
    if (j == NULL) {
        printf("Cannot find job with specified jid\n");
        return;
    } 
    // move the specified job to the foreground
    bring_job_to_fg(j);
} //most recently bged job into fg

void bring_job_to_fg(Job *j) {
    kill(-j->pid, SIGCONT);
    j->status = 0; // set to running
    int status;
    waitpid(-j->pid, &status, WUNTRACED);
}

void bring_job_to_bg(Job *j) {
    printf("[%d] %d\n", j->jid, j->pid);
}

void kill_job(char **args) {
    bool flag = false; // flag for -9
    // invalid cmd
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: kill <job_id>\n");
        return;
    }
    // if there is -9 flag
    if (args[2] != NULL && strcmp(args[1], "-9") == 0) {
        int jid = extract_jid(args[2]);
        // find the job with jid
        Job *j = find_job_jid(job_list, jid);
        if (j == NULL) {
            printf("Cannot find job with specified jid %d\n", jid);
            return;
        } 
        // Send SIGKILL to the specified process
        if (kill(-jid, SIGKILL) == -1) {
            perror("kill");
            return;
        }
        printf("Sent SIGKILL to job %d\n", jid);
    } else { // no flag
        int jid = extract_jid(args[1]);
        // find the job with jid
        Job *j = find_job_jid(job_list, jid);
        if (j == NULL) {
            printf("Cannot find job with specified jid %d\n", jid);
            return;
        } 

        // Send SIGTERM to the process group 
        if (kill(-jid, SIGTERM) == -1) {
            perror("kill");
            return;
        }
        printf("Sent SIGTERM to job %d\n", jid);
    }
}

/*
    File: mysh.c
    Name: Rachel Nguyen, Julia Rieger
    Desc: A simple shell (mysh) that accept lines of text as input and execute 
    programs in response
*/

#include "mysh.h"

void update_joblist(pid_t pid_to_update, int status_to_update) {
	//if joblist doesn't contain job with pid_to_update, return error
	//find job in list
		//if STATUS_TERMINATE, update job status, print to terminal, and remove from list
		//if STATUS_SUSPEND (aka pause a running job) update job status, add to list (?), print jobs
		//if STATUS_RESUME (aka resume suspended job) kill -CONT <pid>, update job status, print jobs

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

void sig_chld_handler(int sig, siginfo_t *info, void *ucontext) {
    block_sigchld(TRUE); //block SIGCHLD
    pid_to_update = info->si_pid;	//store pid and status in globals (critical region)
    sigchld_received = TRUE;	//set sigchld_received flag to update JobLL
    block_sigchld(FALSE); //unblock SIGCHLD
}

void sig_tstp_handler(int signo) {
    sigtstp_received = TRUE;	//set sigtstp_received flag to stop fg job
}

void setup_sighandlers() {
    struct sigaction sa_tstp, sa_chld;
    //set up signal handler for SIGCHLD
    sa_chld.sa_sigaction = sig_chld_handler;     
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_SIGINFO; 
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) { 
	    perror("Error setting up sigchld handler"); 
	    exit(EXIT_FAILURE); 
    }
    // Set up the signal handler for SIGTSTP (Ctrl-Z)
    sa_tstp.sa_handler = sig_tstp_handler;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) { 
	    perror("Error setting up sigtstp handler"); 
	    exit(EXIT_FAILURE); 
    }
}

int main() {
    
    setup_sighandlers();

    while (true) {
		if (sigchld_received) {
            block_sigchld(TRUE); //block SIGCHLD
			update_joblist(pid_to_update, STATUS_TERMINATE); //terminate job with pid_to_update	
            sigchld_received = FALSE; // Reset the flag 	
            block_sigchld(FALSE); //unblock SIGCHLD
		}
        if (sigtstp_received) {
            pid_t curr_foreground_pid = foreground_pid; //set temp pid to current fg job's pid
            kill(curr_foreground_pid, SIGSTOP);  // Send SIGSTOP to the foreground process
            update_joblist(curr_foreground_pid, STATUS_SUSPEND); //suspend job in the foreground
            sigtstp_received = FALSE; // Reset the flag 	
        }

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
        setpgid(0, 0);  // Set the child process to a new process group

        if (execvp(args[0], args) == -1) {
            perror("execvp");
            free_args(args);
            exit(EXIT_FAILURE);
        } 
    } else {
        foreground_pid = pid; //set foreground pid global to be used in case of ctrl z
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

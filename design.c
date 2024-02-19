typedef struct {
    char *cmdline; // the string tokenized by &
    bool bg; 
} Command; // a command separated by &

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
void free_str_array(char **args);
void free_command_array(Command **commands);
void update_joblist(pid_t pid_to_update, int status_to_update); //update JOBLL based on what signals were caught: finds job in LL with pid "pid_to_update", sets status to "status_to_update"
void block_sigchld(int block); //function to block or unblock SIGCHLD- signal masking
void sig_chld_handler(int sig, siginfo_t *info, void *ucontext); //handles SIGCHLD
void sig_tstp_handler(int signo); //handles SIGTSTP
void setup_sighandlers(); //sets up signal handlers with sigaction()

int main() {
    setup_sighandlers();
    bool exit = false;
    
    while (true) {
        if (sigchld_received) { //check sigchld flag
            block_sigchld(TRUE); //block SIGCHLD
			update_joblist(pid_to_update, STATUS_TERMINATE); //terminate job with pid_to_update	
            sigchld_received = FALSE; // Reset the flag 	
            block_sigchld(FALSE); //unblock SIGCHLD
		}
        if (sigtstp_received) { //check sigtstp flag
            pid_t curr_foreground_pid = foreground_pid; //set temp pid to current fg job's pid
            kill(curr_foreground_pid, SIGSTOP);  // Send SIGSTOP to the foreground process
            update_joblist(curr_foreground_pid, STATUS_SUSPEND); //suspend job in the foreground
            sigtstp_received = FALSE; // Reset the flag 	
        }

        //print prompt
        char *input = readline("$ ");

        // check syntax
        if (check_blank_input(input) || !check_syntax(input)) {
            continue;
        }
        
        // tokenize with ; to get a list of sequential commands to be executed
        char **list = tokenize(input, ";");
        
        // for each sequential command, parse to a list of Command structs
        for (each of list[i] in list) {
            Command **commands = parse_to_structs(list[i]); 
            // for each command struct, execute
            for (each of command[j] in commands) {
                char **args = parse_command(commands[j]);
                if (args[0] == built_in) {
                    // handle built_in
                } else {
                    execute_command(args, bg);
                }
            }
        }

    }
}

Command **parse_to_structs(char *list) {
    // tokenize a sequential command with '&' to find the bg processes
    // store them in Command structs
    // return a list of Command structs
}

char **parse_command(Command *command) {
    // DELIM is whitespaces
    // return a list of args that are ready to be parsed into execute_command()
    return tokenize(command->cmdline, DELIM);
}

bool check_syntax(char* input) {
    // check whether input starts with & or ;
    // or contains "&&" "&;" ";&" ";;"
}

void update_joblist(pid_t pid_to_update, int status_to_update) {
    //update JOBLL based on what signals were caught: 
    //finds job in LL with pid "pid_to_update", sets status to "status_to_update"
}

void block_sigchld(int block) {
    //function to block or unblock SIGCHLD- signal masking 
    //(block = 1 to block, 0 to unblock)
}

void sig_chld_handler(int sig, siginfo_t *info, void *ucontext) {
    //handles SIGCHLD by updating flag and globals
}

void sig_tstp_handler(int signo) {
    //handles SIGTSTP by updating flag
}

void setup_sighandlers() {
    //sets up signal handlers (SIGCHLD and SIGTSTP with sigaction())
}

typedef struct {
    char *cmdline; // the string tokenized by &
    bool bg; 
} Command; // a command separated by &

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

int main() {
    while (true) {
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
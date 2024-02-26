Names: Julia Rieger, Rachel Nguyen

Mysh is a simple shell program that accepts lines of text as input and executes programs in response. It provides a basic command-line interface for users to interact with their system.

Shell Structure:
Files:  mysh.c: contains main loop and implements helper functions for mysh.h functions
        mysh.h: header file for mysh.c
        Job.h: header file for Job.c
        Job.c: Implements Job DS and helper functions
        node.h: header file for node.c
        node.c: implements node DS and helper funcs
        JobLL.h: header file for JobLL.c
        JobLL.c: Implements JobLL DS and helper funcs
        Makefile: compile object and exec files

    main control loop
        iter through joblist to handle sig flags
        tokenize by ; to get the order of commands
        tokenize by & to set background flag
        check for builtin functions
        execute each token separated by &
            fork a child process for each command
        break if exit

    -asynchronous approach
    -do not add fg jobs to list (add when suspended)
    
Fully Implemented:
    Foregrounding/terminal
    Process Groups
    Ctrl-C
    Sig-chld handling

Partially Implemented:
        Backgrounding with &: inconsistent segfault when running in the bg and later executing in the fg
        Parsing: segfault/double free memory error when tokenizing with ";"
        Joblist: does not always print correctly the first time (must execute jobs multiple times) 
                not readding jobs to the list correctly- jobs only prints 1 done at a time
        Ctrl-Z: stops the program but gets stuck
        Built-ins: exit works
                    kill does not (bc of previous errors with job_list)
                    fg/bg do not work bc of previous errors with ctrl-z

Testing:
    testJobLL.c -> tests JobLL (works with memory leaks)
        includes testing of all JobLL functions and several Job functions
        
    test cases: ./p -> running in foreground, background, in succession + grouped
                    tested in combination with ls, emacs, jobs
                    
                Signals -> Ctrl-C, Ctrl-Z during both fg and bg processes

                Parsing/Backgrounding with &:
                        - Parsing with "&&", "&;", ";&", ";;" or commands starting with "&" and ";" gives syntax error near delim
                                ./p & ; ls (should be ./p & ls)
                                ./p ;    ; ls
                                & ./p 
                        - Running fg and bg in succession
                                ./p ; ls 
                                ./p 10 & ./p 10 & ./p 10 &
                                ./p ; ./p ; ./p 
                                ./p;./p&./p (no space)

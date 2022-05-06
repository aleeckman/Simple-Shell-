#include "utils.h"

/* Launching Stage */
void executeCommand(struct Cmd_master *c, int *error_code, int *retval) {

    /* @69 tells us that builtin cmd will never be parts of a pipeline. */
    int parse_index = 0;
    char *cmd       = (*c).Parsed_cmds[parse_index].args[0];


    /* First check for built in commands */
    if (!strcmp(cmd, "exit"))
    {
        fprintf(stderr, "Bye...\n");
        fprintf(stderr, "+ completed '%s' [%d]\n", (*c).cmd_input, *retval);
        exit(EXIT_SUCCESS);
    }


    else if (!strcmp(cmd, "cd"))
    {
        *error_code = executeCD((*c).Parsed_cmds[0].args[1]);
        *retval     = errorHandler(error_code);

        fprintf(stderr, "+ completed '%s' [%d]\n", (*c).cmd_input, *retval);
    }


    else if (!strcmp(cmd, "pwd"))
    {           
        *error_code = executePWD();
        *retval     = errorHandler(error_code);

        fprintf(stderr, "+ completed '%s' [%d]\n", (*c).cmd_input, *retval);
    }


    else if (!strcmp(cmd, "sls"))
    {
        *error_code = executeSLS();
        *retval     = errorHandler(error_code);

        fprintf(stderr, "+ completed '%s' [%d]\n", (*c).cmd_input, *retval);
    }


    /* Otherwise, proceed and check other commands */
    else {
        int exit_status[MAX_ARGS+1];
        int *exit_status_ptr = exit_status;

        /* iterate each Programs inside the Parse ... */
        pipeline_recursive(c, exit_status_ptr, c->num_of_parsed-1);

        /*  Print Launching result  */
        fprintf(stderr, "+ completed '%s' ", (*c).cmd_input);
        for(int i=0; i<(*c).num_of_parsed; i++) {
            fprintf(stderr, "[%d]", exit_status[i]);
        }
        fprintf(stderr, "\n");
    }
}



int executeCD(char* dir) {

    // chdir() will return  0 if succeeds to change to a new directory
    if(chdir(dir) == 0) return NO_ERROR;

    // chdir() will return -1 if fails to change to a new directory
    return CANT_CD_INTO_DIR;
}



int executePWD(void) {

    char dir[PATH_MAX]; 
    getcwd(dir, sizeof(dir));

    // print error in pwd if any
    printf("%s\n", dir);

    return NO_ERROR;
}



int executeSLS(void) {

    DIR           *dir;
    struct dirent   *d;
    struct stat      s;

    dir = opendir(".");

    while ((d = readdir(dir)) != NULL) {
        stat((*d).d_name, &s);

        if((*d).d_name[0] != '.')
            printf("%s (%ld bytes)\n", (*d).d_name, s.st_size);
    }

    return NO_ERROR;
}


/* function for processing redirect and append */
void proceed_truncate_or_append(struct Parsed parsed) {
    int redirect_index = 0;

    while (redirect_index < parsed.num_of_redirect) {
        int fd_out;
        int redirect_type = parsed.redirect_type[redirect_index];
        char *file_name   = parsed.output_files[redirect_index++];

        if(redirect_type == TRUNCATE)
            fd_out = open(file_name, O_RDWR|O_CREAT|O_TRUNC, 0644);

        else if(redirect_type == APPEND) {
            fd_out = open(file_name, O_RDWR|O_CREAT|O_APPEND, 0644);
            lseek(fd_out, 0, SEEK_END);
        }

        // direct the output to file_name instead of STDOUT
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}


/* Display the type of error encountered */
int errorHandler(int *error_code) {
    switch(*error_code) {
        
        // Phrasing Error
        case MISSING_CMD: 
            fprintf(stderr, "Error: missing command\n");
            break;
        case TOO_MANY_ARGS:
            fprintf(stderr, "Error: too many process arguments\n");
            break;
        case NO_OUTPUT_FILE:
            fprintf(stderr, "Error: no output file\n");
            break;
        case CANT_OPEN_OUTPUT_FILE:
            fprintf(stderr, "Error: cannot open output file\n");
            break;
        case MIS_LOC_OUTPUT_REDIRECTION:
            fprintf(stderr, "Error: mislocated output redirection\n");
            break;


        // Launching Error
        case CANT_CD_INTO_DIR:
            fprintf(stderr, "Error: cannot cd into directory\n");
            break;
        case CMD_NOT_IMPLEMENT: 
            fprintf(stderr, "Error: command not found\n");
            break;


        // No Any Error
        case NO_LAUNCHING_ERROR: 
            return NO_LAUNCHING_ERROR;
    }

    /* Error detected, the error message is printed */
    fflush(stderr);
    return LAUNCHING_ERROR_EXIST;
}



/* Recursive execvp each cmds inside parse seperated by pipeline 
    Implementation of Redirection are also included.
    Record exit status from child process into exit_status
    
   Work even input command has no pipeline                       */
void pipeline_recursive(struct Cmd_master *c, int *exit_status, int pipe_count) { 

    if (pipe_count < 0) { 
        /* STOP RECURSION HERE */ 
    }

    else 
    {
        int parse_index              = (*c).num_of_parsed - pipe_count - 1;
        struct Parsed parse          = (*c).Parsed_cmds[parse_index];
        char *cmd                    = parse.args[0];

        int redirect_index           = 0;
        int error_code               = 0;
        int status                   = 0;

        /* -1 because we only close fd[1] in parent process */
        int fd[2]; 
        pipe(fd);
        int last_fd [2] = {fd[0] - 1, fd[1] - 1};

        pid_t pid       = fork();
    
        /* Child */
        if (pid == 0) 
        {                
            /*  Pipeline detected   */
            if ((*c).num_of_parsed > 1) {

                // First Parse, i.e. 
                //  1st Parse   in    1st Parse | 2nd Parse | 3rd Parse 
                if(parse_index == 0)
                {  
                    close(fd[0]); 
                    close(last_fd[0]); 
                    close(last_fd[1]);

                    dup2 (fd[1],  STDOUT_FILENO);
                    close(fd[1]);
                }

                // Last Parse, i.e. 
                //  3rd Parse   in    1st Parse | 2nd Parse | 3rd Parse 
                else if (parse_index+1 == (*c).num_of_parsed) 
                {
                    close(fd[0]); 
                    close(fd[1]);
                    close(last_fd[1]);

                    dup2 (last_fd[0], STDIN_FILENO);
                    close(last_fd[0]);
                }

                // Parses between Pipeline, i.e.
                //  2nd Parse   in    1st Parse | 2nd Parse | 3rd Parse 
                else 
                {
                    close(fd[0]);    
                    close(last_fd[1]);

                    dup2(last_fd[0], STDIN_FILENO );
                    dup2(fd[1],      STDOUT_FILENO);
                    close(last_fd[0]); 
                    close(fd[1]);
                }

            }

            /* Check if any redirection and processing them */
            while(redirect_index < parse.num_of_redirect) {
                int redirect_type = parse.redirect_type[redirect_index++];

                // If redirection, open file and write to file.
                if(redirect_type != NO_REDIRECT) 
                    proceed_truncate_or_append(parse);
            }

            execvp(cmd, parse.args);

            /* Launching Error exists, program not executed,
                checking the reason */
            if (errno == ENOENT)       error_code = CMD_NOT_IMPLEMENT; 

            // IMPORTANT! not sure why this is happening in CSIF but
            //  If we have a file named "a" in the directory
            //  When we enter a to my shell, EACCES is returned 
            //  instead of ENOENT.
            // Some Bad implementation here, but we can't think a better
            //  way to tackle this issue
            if (errno == EACCES)       error_code = CMD_NOT_IMPLEMENT;

            if (errno != EACCES && errno != ENOENT) perror("execvp");

            errorHandler(&error_code);

            exit(EXIT_FAILURE);
        }


        /* Parent */
        else if (pid > 0) 
        {
            // Wait return from Child lest it becomes zombie ...
            waitpid(pid, &status, 0);  

            // close write end of current file descriptor, 
            //  we no longer need it to do anything
            close(fd[1]); 

            exit_status[parse_index++] = WEXITSTATUS(status);

            // Go to the next parse
            pipeline_recursive(c, exit_status, --pipe_count);
        }


        /* Error */
        else
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
}


/* check if cmd entered other only has linebreak and whiteSpace */
int nth_being_entered(char *str) {
    int pos = 0;

    if(str[pos] == '\n') return true;

    while(str[pos] != '\0') {
        if (str[pos] != ' ' && str[pos] != '\n') return false;
        pos++;
    }
    
    return true;
}
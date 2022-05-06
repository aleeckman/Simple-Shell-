#ifndef UTILS_H
#define UTILS_H

#include "data_struct.h"
#include <sys/wait.h>

#define PATH_MAX              4096

enum launching_status {
    NO_LAUNCHING_ERROR,
    LAUNCHING_ERROR_EXIST
};

/* execute parsed command */
void executeCommand(struct Cmd_master *c, 
                    int    *error_code, 
                    int    *retval);

/* function for processing truncate and append */
void proceed_truncate_or_append(struct Parsed parsed);

/* handle errors */
int errorHandler(int *ec);

/* built-in commands */
int executeCD(char* dir);
int executePWD(void);
int executeSLS(void);


/* Recursive execvp each cmds inside parse seperated by 
    pipeline 
    Implementation of Redirection are also included.
    Record exit status from child process into exit_status

   Work even input command has no pipeline               */
void pipeline_recursive(struct Cmd_master *c, 
                        int    *exit_status, 
                        int    pipe_count);

/* check if cmd entered only has linebreak and whiteSpace */
int nth_being_entered (char *str);

#endif // UTILS_H

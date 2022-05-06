/*
*  ECS 150 Operating System : Project #1
*  Main program for sshell
*/

#include "utils.h"

int main(void)
{
    while (true)
    {
        struct Cmd_master c;
        char input[MAX_CMDLINE];
        int error_code = NO_ERROR;
        int retval     = NO_LAUNCHING_ERROR;

        /* Print the shell */
        fprintf(stdout, "sshell$ ");
        fgets(input, MAX_CMDLINE, stdin);

         /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO)) {
            printf("%s", input);
            fflush(stdout);
        }

        /* The shell shd only start parsing and launching 
            if command not contain WhiteSpace and LineBreak*/
        if (!(nth_being_entered(input))) {

            /* Phrasing Stage and print Phrasing Error if any*/
            getCommands(&c, input, &error_code);
            retval = errorHandler(&error_code);

            /* No Pharsing Error found, enter the Launch Stage */
            if (error_code == NO_ERROR) {
                executeCommand(&c, &error_code, &retval);
            }

            /* Call the class destrcutor lest Memory Leak */
            cmd_master_destructor(c);
        }
    }

    return EXIT_SUCCESS;
}




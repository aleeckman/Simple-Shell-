/*
*  ECS 150 Operating System : Project #1
*  Implementation of Data Struct
*/
#ifndef HELPER_H
#define HELPER_H

#include <dirent.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

// A program has a maximum of 16 arguments.
#define MAX_ARGS          16

// The maximum length of individual tokens never exceeds 32 characters.
#define MAX_TOKENS        32

// The maximum number of character allowed for command entered by users
#define MAX_CMDLINE      512

// The maximum number of pipes allowed for command entered by users
#define MAX_PIPES          3

/* Redirection Codes */
enum redirection {
    NO_REDIRECT,
    TRUNCATE,
    APPEND
};

/* Error Code        */
enum error_type {
    /* Valid Command */
    NO_ERROR,

    /* Parsing Error */
    TOO_MANY_ARGS,
    MISSING_CMD,
    NO_OUTPUT_FILE,
    CANT_OPEN_OUTPUT_FILE,
    MIS_LOC_OUTPUT_REDIRECTION,

    /* Lanching Error */
    CANT_CD_INTO_DIR,
    CMD_NOT_IMPLEMENT
};

/* Cmd sepeated by pipeline*/
struct Parsed
{
        char **args;
        char **output_files;
        int  num_of_args;
        int  num_of_redirect;
        int  *redirect_type;
        int  *redirect_operator_index;
};


/* Whole command line entered by user*/
struct Cmd_master
{
    char    *cmd_input;
    struct  Parsed *Parsed_cmds;

    // exists a pipe if > 1
    int     num_of_parsed;
};


/* Constrintuctor for class cmd_master */
void getCommands(struct Cmd_master *c,
                 char   input[MAX_CMDLINE],
                 int    *error_code);

/* Constructor for class cmd_master */
struct Parsed parseIndCommand(char *delimited_str,
                              int  *error_code);

/* test if output redirection is correctly implemented */
int test_output_redirection(struct Parsed parsed);

/* check if str entered has character other than "|" and " " */
int present_str_valuable(char *str,
                         int  pos);

/* check if remaining str has character other than ' ' */
int remaining_str_valuable(char *str,
                           int  pos);

/* check if str contains '>' */
int contain_greater(char *str);

/* get the file name of a redirection */
char *get_file_name(char *str,
                    int  pos,
                    int  *checked_len);

/* class destructor for cmd_master */
void cmd_master_destructor(struct Cmd_master c);

/* class destructor for cmd_master */
void parsed_destructor(struct Parsed parsed);

#endif  //HELPER

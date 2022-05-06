#include "data_struct.h"

/* Constructor for class cmd_master */
void getCommands(struct Cmd_master *c, char input[MAX_CMDLINE], int *error_code)
{   
    // There can be maximum MAX_PIPES+1 parsed in the commands
    struct Parsed *Parsed_cmds = malloc((MAX_PIPES+1) * sizeof(struct Parsed));

    // Allocated space each parsed. Maximum of parse = PIPES_MAX + 1 = 4
    for (int i = 0; i < MAX_PIPES + 1; i++) {

        // args in each parsed can have Maximum of 16 arguments.
        Parsed_cmds[i].args = malloc(MAX_ARGS * sizeof(char*));

        // The maximum length of individual tokens never exceeds 32 characters.
        for (int u=0; u < MAX_ARGS; u++) {
            Parsed_cmds[i].args[u] = malloc(MAX_TOKENS * sizeof(char));}
    }

    /* Remove trailing newline from command line */
    char *nl = strchr(input, '\n');
    if (nl)
            *nl = '\0';

    // The maximum number of character allowed for command entered by users is 512
    char *temp = malloc(MAX_CMDLINE * sizeof(char));

    int i              = 0;
    int j              = 0;
    int num_of_parsed  = 0;

    /* start parsing */
    while(input[i] != '\0') {

        /* pipeline discovered */
        if(input[i] == '|') {

            /* redirection or append has been deteced in temporary str there 
                shdn't be a pipeline. Error : Mislocated Output Redirection */ 
            if (contain_greater(temp)) {
                *error_code = MIS_LOC_OUTPUT_REDIRECTION;
                break;
            }

            /* check if temp contains character other than ' ' && '|' */
            if(present_str_valuable(temp, j)) {

                /* temp contains useful character. We shd record it*/
                temp[j] = '\0';
                Parsed_cmds[num_of_parsed] = parseIndCommand(temp, error_code);

                if (*error_code != NO_ERROR) break;
                else {*temp = '\0';  j=0; num_of_parsed++;}
            }

            else { *error_code = MISSING_CMD; break; }
        } // END OF if(input[i] == '|')


        else {
            if (input[i] == ' ') {
                if(present_str_valuable(temp, j))
                    {temp[j] = input[i]; j++;}
            }

            else {temp[j] = input[i]; j++;}
        } // END OF if(input[i] != '|')


        /* Next character is '\0'. End the parsing process */
        if (input[i+1] == '\0') {
            if (present_str_valuable(temp, j)) {
                temp[j] = '\0';
                Parsed_cmds[num_of_parsed] = parseIndCommand(temp, error_code);

                // Test if there is an error for the output redirection
                if(Parsed_cmds[num_of_parsed].redirect_type[0] != NO_REDIRECT) {
                    *error_code = test_output_redirection(Parsed_cmds[num_of_parsed]);

                    if (*error_code != NO_ERROR) break; 
                }
                num_of_parsed  ++;
            } 

            else
                if (*error_code == NO_ERROR) 
                   { *error_code = MISSING_CMD; break; }
        }   // END OF if(input[i+1] == '\0')
        i++;
    }


    /* free those allocated parse not being used */
    for (int k = num_of_parsed  +1; k < MAX_PIPES+1; k++)
    {
        // free c_Str inside args
        for (int u=0; u<MAX_ARGS; u++) {
            free(Parsed_cmds[k].args[u]);
            Parsed_cmds[k].args[u] = NULL;}

        // free args
        free(Parsed_cmds[k].args);
        Parsed_cmds[k].args = NULL;
    }


    struct Cmd_master c_temp = {
        .cmd_input        = input,
        .Parsed_cmds      = Parsed_cmds,
        .num_of_parsed    = num_of_parsed
    };  

    *c = c_temp;
}

/* Constructor for class Parsed*/
struct Parsed parseIndCommand(char *delimited_str, int *error_code)
{
    int i                            = 0;
    int offset                       = 0;
    int args_index                   = 0;
    int files_index                  = 0;
    int num_of_redirect              = 0;

    int *redirect_type               = malloc(MAX_ARGS * sizeof(int));
         redirect_type[0]            = NO_REDIRECT;

    int *redirect_operator_index     = malloc(MAX_ARGS * sizeof(int));
         redirect_operator_index[0]  = -1;

    char **args  = malloc(MAX_ARGS   * sizeof(char*));

    char **files = malloc(MAX_ARGS   * sizeof(char*));

    for(int k = 0; k < MAX_ARGS; k++) {
        args[k]  = malloc(MAX_TOKENS * sizeof(char));
        files[k] = malloc(MAX_TOKENS * sizeof(char));
    }


    /* Starting Parsing */
    while(delimited_str[i] != '\0')
    {
        /* Too many arg and no error has been discoverd*/
        if(args_index >= 16 && *error_code == NO_ERROR) 
            { *error_code = TOO_MANY_ARGS; break; }
        
        /* Discovering a token */
        if(delimited_str[i] != ' ' && delimited_str[i] != '>') 
            { args[args_index][offset++] = delimited_str[i]; }
        
        /* Redirection discovered */
        if(delimited_str[i] == '>') {

            /* There is a token e.g. ls> */  
            if (i>0 && delimited_str[i] != ' ') {
                args[args_index][offset] = '\0';
                args[args_index] = realloc(args[args_index], offset);
                args_index      += remaining_str_valuable(delimited_str, i+1);
                offset           = 0;

                /* '>' is the final useful character */
                if(!remaining_str_valuable(delimited_str, i+1)) 
                    redirect_operator_index[num_of_redirect] = args_index+1;
    
                /* Still sth after '>'              */
                else 
                     redirect_operator_index[num_of_redirect] = args_index;
            }

            /* Check if Append or redirect */
            redirect_type[num_of_redirect]           = 
                                 (delimited_str[i+1] == '>')    ? APPEND : TRUNCATE;
            i += (redirect_type[num_of_redirect]     == APPEND) ?    1   :    0    ;
            num_of_redirect++;

            /* check the file name for redirection */
            int len                  = 0;
            files[files_index++]     = get_file_name(delimited_str, i+1, &len);
            i                       += len;

            /* if there is nothing after the file_name, we shd stop discovering */
            if (!remaining_str_valuable(delimited_str, i+1)) args_index--;
        }

        /* A new token is discovered */
        if(delimited_str[i] == ' ' && offset != 0) {

            args[args_index][offset] = '\0';

            /* reallocate memory designated by malloc */
            /* Otherwises arg[index][offset] will actually
                sth like "str\0--SOME MEMORY ADDRESS OVER HERE--"*/
            args[args_index] = realloc(args[args_index], offset);

            /* check if there are still other token for
                the rest of delimited_str */
            args_index      += remaining_str_valuable(delimited_str, i+1);
            offset           = 0;
        }

        /* Next character is end of line */
        if (delimited_str[i+1] == '\0') {

            /* A token is being discovered and we shd finish it
                before we end the while loop */
            if (offset != 0) {
                args[args_index][offset] = '\0';
                args[args_index]         = realloc(args[args_index], offset);
            }
        }
        i++;
    }

    /* free c_str that are not being used in args      */
    for(int k = args_index+1; k < MAX_ARGS; k++) {
       free(args[k]);
       args[k] = NULL;
    }

    /* free c_str that are not being used in file_name */
    for(int u = files_index ; u < MAX_ARGS; u++) {
       free(files[u]);
       files[u] = NULL;
    }

    struct Parsed parse = {  
        .args                    = args,
        .num_of_args             = args_index+1,
        .output_files            = files,
        .num_of_redirect         = num_of_redirect,
        .redirect_type           = redirect_type,
        .redirect_operator_index = redirect_operator_index
    };

    return parse;
}

/* test if output redirection is correctly implemented */
int test_output_redirection(struct Parsed parsed) {
    int index = 0;

    while (index < parsed.num_of_redirect) {
        struct stat s;
        stat(parsed.output_files[index], &s);
        int   operator_pos  = parsed.redirect_operator_index[index];
        char* output_file   = parsed.output_files[index++];

        /* '>' or '>>' shdn't be in 1st args.  Error : missing Command         */
        if(operator_pos     == 0)               return MISSING_CMD;

        /* No file name is being entered.      Error : no output file          */
        if(output_file      == NULL)            return NO_OUTPUT_FILE;

        /* The file doesn't exist. Not a big deal cause we can create it       */
        if(access(output_file, F_OK) == -1)     return NO_ERROR;

        /* The file cant be opened for write.  Error : cannot open output file */
        if(access(output_file, W_OK) == -1)     return CANT_OPEN_OUTPUT_FILE;

        /* The output_file is a directory.     Error : cannot open output file */
        if(S_ISDIR(s.st_mode))                  return CANT_OPEN_OUTPUT_FILE;
    }

    return NO_ERROR;
}

/* check if str contains '>'*/
int contain_greater(char *str) {
    int pos = 0;
    while (str[pos] != '\0') {
        if (str[pos++] == '>') return true;
    }
    return false;
}

/* check if str entered has character other than "|" and " " */
int present_str_valuable(char *str, int pos) {
    while(pos != 0) {
        if (str[pos] != ' ' && str[pos] != '|') return true;
        pos++;
    }
    return false;
}

/* check if remaining str has character other than ' ' */
int remaining_str_valuable(char *str, int pos) {
    while (str[pos] != '\0') {
        if (str[pos++] != ' ') return true;
    }
    return false;
}

/* get the file name of a redirection */
char *get_file_name(char *str, int pos, int *checked_len) {
    int file_name_index = 0;
    int checked_pos     = pos;
    int start_recording = false;
    int recording_pos   = 0;
    char *file_name     = malloc(MAX_TOKENS * sizeof(char));

    while(str[checked_pos]      != '\0') {
        if (str[checked_pos]    != ' ') {
            start_recording      = true;
            recording_pos        = checked_pos;
        }

        if (start_recording     == true) {
            /* start of file_name is '>' , no file name is entered */
            if(str[checked_pos] == '>' && file_name_index == 0) break;
 
            /* file name has been detected. Stop recording         */
            else if (str[checked_pos] == ' ' || str[checked_pos] == '>')
                          {file_name[file_name_index]   = '\0'; break;}

            /* file name is still discovering. Keep recording      */
            else {file_name[file_name_index++] = str[checked_pos];}
        }
        checked_pos++;
    }
    *checked_len = checked_pos - recording_pos;

    return (file_name_index == 0) ? NULL : file_name;
}

/* class destructor for cmd_master */
void cmd_master_destructor(struct Cmd_master c) {

    // Free all parsed allocated
    for (int i=0; i<c.num_of_parsed; i++) {
        parsed_destructor(c.Parsed_cmds[i]);
    }

    // Now, free the parsed pointer
    free(c.Parsed_cmds);
    c.Parsed_cmds = NULL;
}

/* class destructor for cmd_master */
void parsed_destructor(struct Parsed parsed) {

    // Free c_str inside args
    for (int i=0; i<parsed.num_of_args; i++) {
        free(parsed.args[i]);
        parsed.args[i] = NULL;
    }

    // Then, free args
    free(parsed.args);
    parsed.args = NULL;

    // Free those redirection pointers
    free(parsed.redirect_type);
    free(parsed.redirect_operator_index);
    parsed.redirect_type = NULL;
    parsed.redirect_operator_index = NULL;
}
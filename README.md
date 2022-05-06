#  Simple Shell

## Design of Implementation
The following design choices are listed in their order of implementation.

### Data Structure
The data structure is split into two parts, Cmd_master and Parsed. 

#### Cmd_master
Cmd_master contains within it a 1D char array that holds the entire initial
input entered by the user, an array of the Parsed struct so that each
individual command can be easily accessed from one location, and an integer
that records the number of individual commands entered by the user.

#### Parsed
Parsed contains a 2D array called args to record both the command name and the
arguments passed along with it. It contains another 2D char array meant to hold
the names of any files passed along for redirection (it was implemented as a 2D
array to make future multi-filed redirection possible i.e. "echo world > 
a > b"). The struct also contains three integers, one called num_of_redirect
to help determine how many redirects are called for by a command 
input (again to make future multi-filed redirection possible), redirect_type
to indicate what type of redirection is called for (i.e. truncation or 
appending), and redirect_operator_index to determine after which argument
in the individual command the redirection operator could be found.

#### Summary 
To picture this visually, the below example illustrates a simplified version
of our data structure's organization following input by the user:

~~~
Input: "echo hello world | grep hello | wc -l > file.txt"

Contained within Cmd_master:
- cmd_input:  "echo hello world | grep hello | wc -l > file.txt"
- Parsed 2D array: 

       | Command     |  args                      | output_files |
       |-------------|----------------------------|--------------|
       | Parsed[0]:  | {"echo", "hello", "world"} | {}           |
       | Parsed[1]:  | {"grep", "hello"}          | {}           |
       | Parsed[2]:  | {"wc", "-l"}               | {"file.txt"} |
~~~

Using this sort of implementation, accessing every individual aspect of the
original input is possible by passing one variable of the struct Cmd_master.

### Parsing Input
The parsing process consists of two steps.

#### Step One: '|' Parsing
This first step takes place in the function called `{c} getCommands(...) `. In
this function the initial input is first parsed into individual commands,
separating strings that lie on opposite sides of the character '|'. The
entire command line is passed on to the next stage if '\0' is reached
before a '|' (the user only entered one command). Errors are checked and then, 
for each individual command discovered, we call `{c} parseIndCommand(...)`.
 
 #### Step Two: ' ' and '>' Parsing
In this phase, the individual commands sent by `{c} getCommands(...) ` are
broke down into individual arguments (we use the term arguments here loosely).
Every character that is not a ' ' or '>' is considered to be a part of the 
current indexed argument. When one of these characters are reached, that
argument is "finalized" and, if there are non- spaces / redirection characters
in the rest of the individual command, another argument is initialized
and the process repeats. Only non-whitespace and non-redirection characters
are copied to each argument. 

If a redirection character is discovered, the characters after said characters
are instead copied to the output_files 2D array.

#### Disclaimer (explaining the lack of strtok)
As can probably be observed, both parsing functions make use of one main while
loops and several if statements. This was done both on purpose and by accident.
When we began this process, we were both a bit out of practice with C and,
because of that, we had forgotten about the function `{c} strtok(...)`.
By the time we relearned it, our parsing methods were done and we decided to
keep them for a few reasons. 
First, we believe that they demonstrate a good control over char arrays in C 
and second because we believe they give us more direct control over the char 
arrays actually being handled. Had we wanted to implement further string 
manipulation for potentially future applications of this project, we may have
been restricted by `{c} strtok(...)`. So, while it may have been the easier 
solution we chose to stick with our own. 

### Basic Command Execution
Individual commands are all executed in the same way. A child is created with
fork() and runs execvp(c.parsed[i].args[0], c.parsed[i].args). While this 
method was implemented first, it was inevitably incorporated into
the piping function for reasons discussed later.

### Built-in Commands
These commands were kept separate from the main pipeline / command
execution function as they will not be redirected or piped. Instead 
they, except for exit, have all been given their own special functions,
i.e. `{c} executeCD()`. These commands are run and their results are 
passed to `{c} errorHandler()`, more on that later.

### Output Redirection
Output redirection occurs after piping, specifically on either the last command
identified by the piping function or, if no pipes exist, on the only command
from the input. A while loop checks the num_of_redirects to determine
if the command contains a pipe, if it does, redirection proceeds. 

Note: file opening is checked in the parsing stage to save time. 

With that, the program calls `{c} proceed_truncate_or_append(...)` which
checks if the file specified will be appended to (>>) or truncated (>). Output
from the _STDOUT_ originally generated by the command is then stored 
in said file. Errors are checked once again and reported if necessary. 

### Piping
Originally, we attempted to implement a version of the Professor's Pseudo Code
from the Syscall Lecture Slides (slide 37). However, while it proved successful
in a limited capacity, it was hard to use in the implementation of multiple 
pipes, specifically when you had an odd number of commands. We also found 
it difficult to receive the _WEXITSTATUS_ message back. An attempt was
made to remedy these problems but we ended up with a buggy program and 
several nested if loops which lowered the quality of our project.

We needed some way to take what we learned from both attempts and implement
a method whereby essentially we only had to worry about one fork() at a time. 
We also needed said method to capable of consistently identifying the
previous command's file descriptors while working on the current one.

Ultimately, we settled on a recursive function that would do just that. 
Due to its recursive nature, we only really had to worry about one parent 
and one child process at a time. 

The parent process would be responsible for recording the exit status of 
the child process in an array (one error code for each command) and
for handling the all future recursion. 

The child process would be responsible for all the pipeline logic and command 
execution. It would also handle redirection after the final command was 
identified as only the final command can redirect in a pipeline. The logic of 
this process is summarized below:

- If parsed[parse_index] is the first in the pipeline: set output to fd[1]
- If it is the middle of the pipeline, get input from last command's fd[0] and
set output to fd[1].
- If it is the last command in the pipeline, get last command's fd[0]
- Now check for redirection and redirect if necessary
- Finally execute commands and catch any errors if they exist.

Note: To get the last command's file descriptors:
~~~
int fd[2]; // Current file descriptors
pipe(fd);
last_fd[2] = {fd[0]-1, fd[1]-1} // Previous file descriptors
~~~
This works as in the parent process we only close fd[1]. We don't close the
fd[0] as our _next_ child process will need to link it with _STDIN_.

### Error Handling
Error handling was implemented with the use of enums. Each potential error
specified by the prompt was given its own identifier (enum element) to
improve readability. Almost every function in our program keeps track of the
current command's error status, if an error is identified in that function,this
allows us to quickly pass it off to `{c} errorHandler(...)` and display the 
necessary messages and exit values.

## Coding Style Choices

### Pass by Pointer
In `{c} getCommands(...)` and several other functions throughout our program 
we ended up passing variables by pointer. This was chosen as often times 
we needed to edit multiple variables in a single function and we needed to 
save their resulting values. Had we used pass by value it would have made
implementation much more difficult, especially, say, in the case of functions
like `{c} executeCommand(...)` where we needed to both modify the error 
code for error handling and the return value (retval) for later use. 

### Macros and Enums
We used Macros and Enums to primarily increase readability and also group 
similar variables together (i.e. for error handling / redirection types). 

## File Description
~~~
| File Name     | Usage                                                       |
| ------------- |-------------------------------------------------------------|
| data_struct   | To establish Cmd_master through the parsing of console input|
| utils         | To handle all command execution and error handling          |
| sshell        | To serve as the hub for data collection and function calls  |
~~~

## Shell Visualized
~~~
Do Forever (until exit is called):
    // Receive Input
    // Parse input and save said parses into data struct
    // Check for parsing errors
    // If no such parsing errors exist:
        // Run commands
           // If Builtin: exit, sls, pwd, cd
           // Otherwise: Execute external commands, redirect if necessary
        // Check for errors
~~~
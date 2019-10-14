sshell Report

This program is a simple shell that handles UNIX commands. The shell's jobs
include printing the prompt, reading in user input, and executing the commands
that were given by the user. Also it prints out the appropriate messages about
the command that was executed.

A major part of this program is parsing user input. Since there is a lot of
variation in UNIX commands, we designed our parsing function to iterate
character by character through the input. We preferred the idea of looping
through the input instead of using strtok because there is many variations
so using strtok could make the code much more complicated due to an excess
of spaces or piping commands. So when parsing, we loop through the input and
when we find a space, pipe or input/output redirection then we store the
previous sequence of characters to keep track of commands and arguments. In some
cases, when we find a space, pipe or redirection there will be no previous
sequence of characters; this means that there was a surplus of whitespace or
there was a pipe.

In order for the shell to be effective we implemented a linked list to store a
struct which holds information for each command. If there is piping in the user
input, then more nodes will be added to the linked list so ultimately there will
be a node for every command throughout the user input.
For example, if a command was "echo hello | tr i o" :
There would be two nodes in the linked list, the first being the "echo" command
and the second being the "tr" command. So when parsing if we find a pipe then
we add a new node to the linked list and then start adding any arguments found
into the new node. This cycle repeats as long as there are more pipes within the
user input. If a command does not have any pipes then there will always be one
node in the linked list. We found this method useful since we do not know how
many pipes there will be and it made it simple to add new arguments into a
struct while parsing.

In dealing with input and output redirection, we have two strings inside our
struct; one stores an input file and the other stores a output file if they
exist. This makes it easy to access the particular file when executing the
commands. Overall the data structure used was a linked list and each node is
a struct that holds all the information necessary to execute a command.

So I have discussed how we store and parse all the user input but now I will
show how we execute the commands. In the case where cmd->next is NULL (meaning
the user input is just a single command), it will just call
execvp(cmd->exec, cmd->args); cmd->exec is the command and cmd->args is all the
arguments used. When there is a pipe, then cmd->next will point to another
struct that holds all the information about the next command. The process then
forks and it changes the input and output of the two processes to make the pipe
successful. In order to keep track of the return values of each process, in our
struct we have a variable that stores this return value. So in main when
printing out the return value, we just go through each struct and print their
return values.

When there is input/output redirection, we check if we stored a file name in our
input file variable or our output file variable. If we did then we change the
input or output of the process using file descriptors.

The are builtin commands are defined in main. cd checks to see if the
directory name exists and if it is then it changes directory. pwd prints the
directory. exit checks to see if there are any processes running in the
background, if not then it exits the shell.

In dealing with background processes, when we parse and find a '&' then we
set a boolean to true. In main if that boolean is true then we store the command
in a temporary struct that holds the background processes. Then the sshell moves
on and will just return the result of command once it is finished. We thought
making a separate struct for background processes would make the code simpler
and easier to make sure it works correctly. Once a background is completed we
remove the command from the temp struct. So as long as there are background
commands in the temporary struct they will continue to run until the complete.

Error handling was also a major part of this project. Most of the error handling
was dealt with in the parsing function. The parsing function would parse the
user input and attempt to insert a command or an argument into a struct. When
these commands or arguments were being inserted we would test to make sure there
were no syntactical errors. If there were any errors, then the correct error
output would be printed. If there was an error about a file not existing or a
command not existing, then those errors would be printed out in main. Those
errors would be determined by the return value of the commands being executed.

In order to make debugging as easy as possible, we tested as we completed each
different phase. The parsing took the most amount of time to test just because
there is many possible variations of user input. Also we used the sample tester
shell script and that was useful in making sure that each error or result was
being outputted correctly.

Overall our design was to implement a linked list to hold a struct that held
any necessary information about a command to have it be executed correctly.
Each node would be a separate command to deal with pipes and if there was
redirection the file name would be stored in the struct. This would allow our
shell to handle user input and execute it effectively. 


# simple_shell
A simple shell designed as an abstraction layer between user input and system level calls 

This shell supports input/output redirection, chained pipelining, error handling, yet background processes still being worked on.

We broke the project features up into smaller problems and worked our way through them one by one.

The hardest parts were figuring out how to redirect the error output from the child to be handled by the parent process and handling background processes in general. 

To run our program:

1. make
2. ./simple_shell [-n]

[-n] will disable the prompt "shell :" but will still accept command input.

ctrl-c in the shell will not terminate the shell, but will terminate child processes and prompt for another command in our shell.
ctrl-d will terminate our program



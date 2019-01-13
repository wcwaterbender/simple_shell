// Samuel Cornish 
// Michael Amalfitano

#include <stdlib.h>   // needed to define exit()
#include <unistd.h>   // needed to define getpid()
#include <stdio.h>    // needed for printf() 

int main(int argc, char **argv) {
  char *args[] = {"cat", "1", 0};   // each element represents a command line argument
  // char *args[] = {"lst", 0};     // each element represents a command line argument
  // char *args[] = {"echo", "$PATH", 0};    // each element represents a command line argument
  char *env[] = {0};                // leave the environment list null

  //printf("About to run cat 1\n");
  execvp("cat", args);
  // execvp("lst", args);
  // execvp("echo", args);
  perror("ERROR: ");            // if we get here, execvp failed
  // execve("cat", args, env);
  // perror("execve");          // if we get here, execvp failed
  exit(1);
}
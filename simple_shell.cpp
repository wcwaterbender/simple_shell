// Samuel Cornish 
// Michael Amalfitano

#include <stdlib.h>   // needed to define exit()
#include <unistd.h>   // needed to define getpid() and getopt()
#include <stdio.h>    // needed for printf() 
#include <string.h>
#include <sys/wait.h>

using namespace std;

#define BUFFERSIZE 512

char **split_input(char *input){
	char **command = (char **)malloc(512 * sizeof(char *));
	char *parsed;
	int index = 0;
	parsed = strtok(input, " \n");
        while (parsed != NULL) {
        	command[index] = parsed;
            	index++;
		parsed = strtok(NULL, " \n");
        } 

    command[index] = NULL;
    return command;
}

void shell_loop(int flag){
	char line[BUFFERSIZE];
	char **split;
	char *cmd;
	pid_t child_pid;
	int stat_loc;
	do{
		if(!flag)
			printf("shell: ");
		fgets(line,BUFFERSIZE,stdin);
		split = split_input(line);
		
		cmd = split[0];
		child_pid = fork();
		
		if (child_pid == 0) {
			printf("%s",args[1]);
			execvp(cmd, split);
			printf("this is a failure\n");

		} else if (child_pid > 0) {
			//parent process
			waitpid(child_pid, &stat_loc, WUNTRACED);
		} else {
			//fork failed
			printf("fork failed\n");
		}
	} while(!feof(stdin));
}




int main(int argc, char **argv) {
  //check for -n command passing
  int flag, opt;
  flag = 0;
  while ((opt = getopt(argc, argv, "n")) != -1) {
	switch (opt) {
	case 'n':
		flag = 1;
		break;
	default:
		break;
	}
   }
  
   shell_loop(flag);
 
  
  //char *args[] = {"cat", "1", 0};   // each element represents a command line argument
  // char *args[] = {"lst", 0};     // each element represents a command line argument
  // char *args[] = {"echo", "$PATH", 0};    // each element represents a command line argument
  //char *env[] = {0};                // leave the environment list null

  //printf("About to run cat 1\n");
  //execvp("cat", args);
  // execvp("lst", args);
  // execvp("echo", args);
  //perror("ERROR: ");            // if we get here, execvp failed
  // execve("cat", args, env);
  // perror("execve");          // if we get here, execvp failed
  exit(1);
}

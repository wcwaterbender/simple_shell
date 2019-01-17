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

char *clean_input(char *input){
	char * clean = (char *)malloc(1024 * sizeof(char));
	int j = 1;
	clean[0] = input[0];
	for( int i = 1; input[i]!='\0'; ++i){
		if((input[i] == '<') || (input[i] == '>') || (input[i] == '|') || (input[i] == '<')){
			clean[j] = ' ';
			clean[j+1] = input[i];
			clean[j+2] = ' ';
			j = j+3;
		}
		else{
			clean[j] = input[i];
			j = j+1;
		}
	}
	clean[j] = '\0';
	return clean;
}
	

void shell_loop(int flag){
	char line[BUFFERSIZE];
	char **split;
	char *cmd;
	char *test;
	pid_t child_pid;
	int stat_loc;
	do{
		if(!flag)
			printf("shell: ");
		fgets(line,BUFFERSIZE,stdin);
		split = split_input(line);
		test = clean_input(line);
		cmd = split[0];
		child_pid = fork();
		
		if (child_pid == 0) {
			execvp(cmd, split);
			perror("ERROR: ");
		} else if (child_pid > 0) {
			//parent process
			waitpid(child_pid, &stat_loc, WUNTRACED);
		} else {
			//fork failed
			printf("fork failed\n");
		}
		bzero(line,BUFFERSIZE);
		fflush(stdin);
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

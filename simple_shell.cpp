// Samuel Cornish 
// Michael Amalfitano

#include <stdlib.h>   // needed to define exit()
#include <unistd.h>   // needed to define getpid() and getopt()
#include <stdio.h>    // needed for printf() 
#include <string.h>
#include <sys/wait.h>

using namespace std;

#define BUFFERSIZE 512

void shell_loop(int flag){
	char line[BUFFERSIZE];
	char **args;
	int signal;
	do{
		if(!flag)
			printf("shell: ");
		fgets(line,BUFFERSIZE,stdin);
		//args = split_input(line);
		//signal = prog_execute(args);
		printf("%s",line);
		
	} while(!feof(stdin));
}


//char **split_input(char * input){








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

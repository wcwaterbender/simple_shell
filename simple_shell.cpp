// Samuel Cornish 
// Michael Amalfitano

#include <ctype.h>
#include <stdlib.h>   // needed to define exit()
#include <unistd.h>   // needed to define getpid() and getopt()
#include <stdio.h>    // needed for printf() 
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace std;

#define BUFFERSIZE 512

static char* args[512];
pid_t pid;
int status;
int command_pipe[2];
static int n = 0;
static char line1[1024];
static char* line;
#define READ  0
#define WRITE 1

void func(int signum) 
{ 
    wait(NULL); 
}

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

static char* skipwhite(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}
//split the command and it's arguments 
static void split(char* cmd)
{
	cmd = skipwhite(cmd);
	char* next = strchr(cmd, ' ');
	int i = 0;
 
	while(next != NULL) {
		next[0] = '\0';
		args[i] = cmd;
		++i;
		cmd = skipwhite(next + 1);
		next = strchr(cmd, ' ');
	}
 	//if command is not empty
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
 
	args[i] = NULL;
	
}

//run the command
static int command(int input,int output, int first, int last, int bg, int redir)
{
	int pipettes[2];
 
	pipe( pipettes );
		
	pid = fork();

	if (pid == 0) {//child
		if (first == 1 && last == 0 && input == 0) {
			// First command
			dup2( pipettes[WRITE], STDOUT_FILENO );
		} else if (redir != 0) {
			dup2(input, STDIN_FILENO);
			dup2( pipettes[WRITE], STDOUT_FILENO );
		} else if (first == 0 && last == 0 && input != 0) {
			// middle commands
			dup2(input, STDIN_FILENO);
			dup2(pipettes[WRITE], STDOUT_FILENO);
		} else if (last ==1  && output != 0){//last command with output redir
		   	dup2( input, 0);
		       	dup2(output, 1);
		} else {
			// last command
			dup2( input, STDIN_FILENO );
		}
		if (execvp( args[0], args) == -1)
			_exit(EXIT_FAILURE); // If child fails
	}
	else{//parent
		if(!(bg)){
			waitpid(pid,&status,WUNTRACED);
		}else{
			signal(SIGCHLD,func);
		}
	}
	if (input != 0) 
		close(input);
 
	// Nothing more needs to be written
	close(pipettes[WRITE]);
 
	// If it's the last command, nothing more needs to be read
	if  (last == 1)
		close(pipettes[READ]);
 
	return pipettes[READ];	
}

static void cleanup(int n)
{
	int i;
	for (i = 0; i < n; ++i)
		wait(NULL);
}

static int run(char* cmd, int input, int output, int first, int last, int bg, int redir)
{
	split(cmd);
	if (args[0] != NULL) {
		if (strcmp(args[0], "exit") == 0) 
			exit(0);
		n += 1;
		return command(input,output, first, last, bg, redir);
	}
	return 0;
}

char *clean_input(char *input){
	char * clean = (char *)malloc(1024 * sizeof(char));
	int j = 1;
	clean[0] = input[0];
	for( int i = 1; input[i]!='\0'; ++i){
		if((input[i] == '<') || (input[i] == '>') || (input[i] == '|') || (input[i] == '&')){
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
	
	do{
		if(!flag)
			printf("shell: ");

		fgets(line1,BUFFERSIZE,stdin);
		int bgFlag = 0;
		//check bg process and strip the &
		if(strchr(line1,'&') != NULL){
			bgFlag = 1;
			//trim off the &
			line1[strlen(line1)-2] = 0;
		}
		line = clean_input(line1);
		char* cmd = line;
		char *inputFile = NULL;
		char *outputFile = NULL;
		char rebuild[1024];
		char* inputRedir = strchr(cmd, '<');
		char* outputRedir = strchr(cmd, '>');
		char* pipeLoc = strchr(cmd,'|');
		
	        int input, output;
		int redir = 0;	
		


		if(inputRedir!=NULL){
 			*inputRedir = '\0';
 			inputFile = strtok(inputRedir + 1, " \t\n");
 			if((open(inputFile, O_RDONLY))<0){
 				perror("ERROR: ");
				exit(-1);
			}
 			input = open(inputFile, O_RDONLY);
			if(pipeLoc != NULL){
				strcpy(rebuild,cmd);
				strcat(rebuild,pipeLoc);
				cmd = rebuild;
			}
 		}
		else{
 			input = 0;
		}
		
			
		
		if(outputRedir !=NULL) {
			*outputRedir = '\0';
 			outputFile = strtok(outputRedir + 1, " \t\n");
 			output = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
 		}
 		else
 			output = 0;
		
		
		int first = 1;

			
		char* next = strchr(cmd, '|');
		if(next != NULL){
			redir = 1;
		}
	        //go through command list	
		while (next != NULL) {
			*next = '\0';
			input = run(cmd, input, output, first, 0, bgFlag, redir);
 
			cmd = next + 1;
			next = strchr(cmd, '|'); // Find next '|' if present
			first = 0;
			redir = 0;
		}
		first = 0;
		input = run(cmd, input, output, first, 1, bgFlag,redir);
	
		if (input!=0){
			close(input);
		}
		if (output!=0){
			close(output);
		}
		cleanup(n);
		n = 0;
		
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
 	return(0);

}

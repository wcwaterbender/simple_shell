//Compile with g++

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
static char* args[512];
pid_t pid;
int command_pipe[2];
 
#define READ  0
#define WRITE 1

static void split(char* cmd);
static int command(int input, int output, int first);
static char* skipwhite(char* s);
static int run(char* cmd, int input, int output, int first);
static void cleanup(int n);
static char line[1024];
static int n = 0; // number of calls to 'command'
 
int main()
{
	while (1) {
		// Print the prompt
		printf("$$ ");
		fflush(NULL);
 
		// Read a command line
		if (!fgets(line, 1024, stdin)) 
			return 0;

		char *inputFile = NULL;
		char *outputFile = NULL;
		char* cmd = line;
 		char* inputRedir = strchr(cmd, '<'); // find input redir if present
 		char* outputRedir = strchr(cmd, '>'); // find output redir if present
 		char* appendRedir = strstr(cmd, ">>"); //find append redir if present
 		int input,output;
 		if(inputRedir!=NULL){
 			*inputRedir = '\0';
 			inputFile = strtok(inputRedir + 1, " \t\n");
 			if((open(inputFile, O_RDONLY))<0)
 				exit(-1);
 			input = open(inputFile, O_RDONLY);
 		}
 		else
 			input = 0;

 		if(appendRedir!=NULL){
 			*appendRedir = '\0';
 			outputFile = strtok(appendRedir + 2, " \t\n");
 			output = open(outputFile, O_CREAT | O_RDWR | O_APPEND,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
 		}
 		else if(appendRedir==NULL && outputRedir!=NULL){
 			*outputRedir = '\0';
 			outputFile = strtok(outputRedir + 1, " \t\n");
 			output = open(outputFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
 		}
 		else
 			output = 0; 
		
		int first = 1;
 
		char* next = strchr(cmd, '|'); // Find first '|' if present
 
	    // check if a pipe is present and execute this if block
		if (next!=NULL){
			*next = '\0';
			input = run(cmd,input,output,first);
			cmd = next+1;
			first = 0;
		}
		else
			first=0; 
		input = run(cmd, input,output,first);
		if (input!=0){
			close(input);
		}
		if (output!=0){
			close(output);
		}
		cleanup(n);
		n = 0;
	}
	return 0;
}
 
static int run(char* cmd, int input,int output, int first)
{
	split(cmd);
	if (args[0] != NULL) {
		if (strcmp(args[0], "exit") == 0) 
			exit(0);
		n += 1;
		return command(input, output, first);
	}
	return 0;
}
 
// skip whitespaces in the command
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
 
	if (cmd[0] != '\0') {
		args[i] = cmd;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
 
	args[i] = NULL;
}

//run the command
static int command(int input, int output, int first)
{
	int pipettes[2];
 
	pipe( pipettes );	
	pid = fork();
 
	if (pid == 0) {
		if (first == 1) {
			// First command
			dup2( pipettes[WRITE], STDOUT_FILENO );
		}else {
			// Second command
			if (output==0) {
			dup2( input, STDIN_FILENO ); //display on prompt
			}else {
				// Second command
				dup2( input, 0);
				dup2(output, 1);
			}
		}
 
		if (execvp( args[0], args) == -1)
			_exit(EXIT_FAILURE); // If child fails
	}
 
	if (input != 0) 
		close(input);
 
	// Nothing more needs to be written
	close(pipettes[WRITE]);
 
	// If it's the Second command, nothing more needs to be read
	if (first == 0)
		close(pipettes[READ]);
 
	return pipettes[READ];
}

static void cleanup(int n)
{
	int i;
	for (i = 0; i < n; ++i) 
		wait(NULL); 
}

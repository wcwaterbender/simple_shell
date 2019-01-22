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
#include <errno.h>

using namespace std;

#define BUFFERSIZE 512

static char* args[512];
pid_t w;
int status, sid;
int command_pipe[2];
static int n = 0;
static char line1[1024];
static char* line;
char errbuf[1024];
#define READ  0
#define WRITE 1

void func(int signum) 
{ 
    wait(NULL); 
}

int cd(char **args){
	if (chdir(args[1]) != 0){
		perror("ERROR");
	}
	return 1;
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
 	int errorno[2];
	pipe(errorno);
	pipe(pipettes);
	
	
	signal(SIGINT, NULL);
	pid_t wpid;
	pid_t pid = fork();
			
	if( pid == -1)
		perror("ERROR");
	else if (pid == 0) {//child
		dup2(errorno[1],2); // capture error of child
		close(errorno[1]);
		if(bg ==1){
			sid = setsid();
		}	
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
		if (execvp( args[0], args) == -1){
			perror("");
			exit(-1);
		}
	}
	else{//parent
		if(!bg){
			 wpid = waitpid(pid, &status, 0);
			 if(wpid != pid)
				fprintf(stderr, "ERROR: %s: %s\n", args[0], strerror(errno));
			 if (WIFEXITED(status) && WEXITSTATUS( status ) != 0){
				char buffer[1024];
				bzero(buffer,1024);
		   		close(errorno[1]);
		  		while(read(errorno[0], buffer, sizeof(buffer)) != 0){
		   		}
				close(errorno[0]);	
				if(WEXITSTATUS( status ) > 1)
					fprintf(stderr, "ERROR: %s : %s",args[0], buffer);
				else
					fprintf(stderr, "ERROR: %s", buffer);
				bzero(buffer,1024);
			 }

		}
		else{
			waitpid(pid, &status, WNOHANG | WCONTINUED);
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


static int run(char* cmd, int input, int output, int first, int last, int bg, int redir)
{       
	
	split(cmd);
	
	if (args[0] != NULL) {
		if (strcmp(args[0], "exit") == 0) 
			exit(0);
		if (strcmp(args[0], "cd") == 0){
			if(args[1] == NULL)
				args[1] = getenv("HOME");
			cd(args);
			return 0;
		}
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
		if((input[i] == '<') || (input[i] == '>') || (input[i] == '|')){
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
	signal(SIGINT, SIG_IGN);

	do{
		if(!flag)
			printf("shell: ");

		fgets(line1,BUFFERSIZE,stdin);
		int bgFlag = 0;
		//check bg process and strip the &
		if(strchr(line1,'&') != NULL){
			bgFlag = 1;
			//trim off the &
			line1[strlen(line1)-2] = '\n';
		}
		
		line = clean_input(line1);
		
		char* cmd = line;
		char *inputFile = NULL;
		char *outputFile = NULL;
		char rebuild[1024];
		char* inputRedir = strchr(cmd, '<');
		char* pipeLoc = strchr(cmd,'|');
		char* outputRedir = strchr(cmd, '>');
	        int input, output;
		int redir = 0;	
		


		if(inputRedir!=NULL){
 			*inputRedir = '\0';
 			inputFile = strtok(inputRedir + 1, " \t\n");
 			if((open(inputFile, O_RDONLY))<0){
 				perror("ERROR");
				continue;
			}
 			input = open(inputFile, O_RDONLY);
			if(pipeLoc != NULL){
				strcpy(rebuild,cmd);
				strcat(rebuild,pipeLoc);
				cmd = rebuild;
			}
			else if(outputRedir != NULL){
				strcpy(rebuild,cmd);
				strcat(rebuild,outputRedir);
				cmd = rebuild;
			}
 		}
		else{
 			input = 0;
		}
		
			
			
		outputRedir = strchr(cmd, '>');
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
		//cleanup(n);
		n = 0;
		
		bzero(line,BUFFERSIZE);
		
		fflush(stdin);
		bgFlag = 0 ;
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
	fflush(stdin);
	fflush(stdout);	
 	return(0);

}

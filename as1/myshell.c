#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

extern char **getline();

void executeArgs(char **arguments, int previousfd[2], bool isPiped)
{
	pid_t pid;
    int status, wfd, rfd, i, pipingPos;
    int currentfd[2];
    bool piped = false, redirectTo = false, redirectFrom = false;
 	char * fileOut, * fileIn;

    for(i = 0; arguments[i] != NULL; i++) 
	 {
		 printf("Argument %d: %s\n", i, arguments[i]);
		 
		 if(strcmp(arguments[i],"exit") == 0)
		 {
		  exit(0);
		 }

		 else if(strcmp(arguments[i], "cd") == 0)
		 {

		 	int success = 0;
		 	if(arguments[i+1] == NULL)
		 		success = chdir("~");
		 	else if((success = chdir(arguments[i+1])) == -1)
		 	{
		 		printf("Error using the cd command\n");
		 		exit(1);
		 	}
		 	return;
		 }
		 
		 else if (strcmp(arguments[i],"<") == 0) 
		 {
	 		redirectFrom = true;
	 		fileIn = arguments[i+1];
	 		arguments[i] = NULL;
		 }
		 else if (strcmp(arguments[i],">") == 0) 
		 {
	 		redirectTo = true;
	 		fileOut = arguments[i+1];
	 		arguments[i] = NULL;
		 }
		 
		 else if(strcmp(arguments[i],"|") == 0)
		 {
		 	piped = true;
		 	arguments[i] = NULL;
		 	pipingPos = i+1;
		 	if(pipe(currentfd) < 0)
		 	{
		 		printf("System call pipe failed\n");
		 		exit(1);
		 	}
		 	break;
		 }
 	 }

     if ((pid = fork()) < 0) 
     {     /* fork a child process           */
          printf("Forking child process failed\n");
          exit(1);
     }
     else if (pid == 0) 
     {    
          if(redirectTo)
          {
          	if((wfd = open(fileOut,O_CREAT | O_WRONLY, 00700)) < 0)
          	{	
          		perror("Output redirect error: ");
	            exit(1);	
          	}
            if(close(STDOUT_FILENO) < 0)
            {
            	perror("System call close() error: ");
            	close(wfd);
	            exit(1);
            }
          	if(dup(wfd) != STDOUT_FILENO)
          	{
          		perror("System call dup() error: ");
          		close(wfd);
	            exit(1);
          	}
          }
          else if(redirectFrom)
          {
          	if((rfd = open(fileIn, O_RDONLY)) < 0)
          	{
          		perror("Input redirect error: ");
          		exit(1);
          	}
          	if(close(STDIN_FILENO) < 0)
            {
            	perror("System call close() error: ");
            	close(rfd);
	            exit(1);
            }
          	if(dup(rfd) != STDIN_FILENO)
          	{
          		perror("System call dup() error: ");
          		close(rfd);
	            exit(1);
          	}
          }
          else if(piped || isPiped)
          {
          	if(isPiped)
          	{
          	if(close(STDIN_FILENO) < 0)
            {
            	perror("System call close() error: ");            	
	            exit(1);
            }
          	if(dup(previousfd[0]) != STDIN_FILENO)
          	{
          		perror("System call dup() error: ");          		
	            exit(1);
          	}
				close(previousfd[0]);
	        	close(previousfd[1]);
	        }
	        if(piped)
	        {
	        	if(close(STDOUT_FILENO) < 0)
            	{
            		perror("System call close() error: ");            	
	            	exit(1);
            	}
	          	if(dup(currentfd[1]) != STDOUT_FILENO)
	          	{
	          		perror("System call dup() error: ");          		
		            exit(1);
	          	}
					close(currentfd[0]);
		        	close(currentfd[1]);
				}	        
          }

          if (execvp(arguments[0], arguments) < 0) 
	      {     
	      	  perror("Execvp");
	      	  exit(1);
	      }
     }
     else 
     {                                  /* for the parent:      */
        while (wait(&status) != pid);       /* wait for child completion  */
        if(piped)
        {
        	close(currentfd[1]);        	
        	executeArgs(&arguments[pipingPos], currentfd, piped);
        }
     }
}

int main() 
{
 char **args;
 
 while(1) 
 {
	 args = getline();
 	 executeArgs(args, NULL, false);
 }
}
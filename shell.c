#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

#include "definitions.h"

int main(int argc, char **argv, char **envp)
{
   int i;
   char c;
   char temp[MAX_LEN]; //commandLength + maxArgs * maxArgLength
   
	//signal(SIGINT, SIG_IGN); //ignores SIGINT signal
	signal(SIGINT, handle_signal); //SIGINT = signal for CTRL-C
	initializeEnv(envp);
	initializePaths();
	
	bzero(temp, MAX_LEN);
	printf("%s ", SHELL_TAG);
	
	//Shell will be terminated when getchar() returns end-of-file (CTRL-D)
	while(c != EOF)
	{
		c = getchar();
		if(c == '\n')
		{
		   processCommand(temp);
			printf("%s ", SHELL_TAG);
			
			/*free and unset memory*/
			freeArgs();
			bzero(temp, MAX_LEN);
	   }
	   else
	   {
	      strncat(temp, &c, (size_t) 1);
	   }
	}
	
	/*free up memory*/
	for(i = 0; shellArgs[i] != NULL; i++) free(shellArgs[i]);
	for(i = 0; shellEnv[i] != NULL; i++) free(shellEnv[i]);
	for(i = 0; shellPaths[i] != NULL; i++) free(shellPaths[i]);
	
	printf("\n");
	return 0;
}

/* Avi's psuedocode added on Nov 24
 while (program is running)
 {
 //initialize timer for process
 int processtimer = 0;
 //CPUMAX=99:200 (programs can run at most 200 seconds with CPU utilization of 99% or more.
 if (CPUMAX == 99 && processtimer == 200)
 {
 //kill process
 
 }
 //MEMMAX=50M:360 (programs cannot exceede 50Megabytes for more than 360 seconds)
 if (MEMMAX == 50000000 && processtimer == 360)
 {
 //kill process
 }
 //TIMEMAX=250 (programs can’t run for more than 250 seconds)
 if (TIMEMAX == 250)
 {
 //kill process
 }
 }
*/

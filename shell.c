#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define CMD_LEN 100

typedef void (*sighandler_t)(int);

char c = '\0';
char* SHELL_TAG = "[SHELL v0.1]";

void execute(char *cmd)
{
	int i;
	if(fork() == 0) {
	
      /*UNDER CONSTRUCTION*/
      /*This command works, but takes no parameters or environment variables*/
		i = execlp(cmd, "", NULL);
		/*END OF CONSTRUCTION*/
		
		if(i < 0)
		{
			printf("%s: %s\n", cmd, "*command not found*");
			exit(1);		
		}
	} else {
		wait(NULL);
	}
}

void handle_signal(int signo) //Handler for CTRL-C signal
{
	printf("\n%s ", SHELL_TAG);
	fflush(stdout); //flush output to stdout
}

int main(int numArgs, char **args, char **environment)
{
   char c;
   char *cmd = malloc(sizeof(char) * CMD_LEN);
   
	//signal(SIGINT, SIG_IGN); //ignores SIGINT signal
	signal(SIGINT, handle_signal); //SIGINT = signal for CTRL-C
	
	printf("%s ", SHELL_TAG);
	//Shell will be terminated when getchar() returns end-of-file (CTRL-D)
	while(c != EOF)
	{
		c = getchar();
		if(c == '\n')
		{
		   //printf("Operations start here... \n");
		   execute(cmd);
			printf("%s ", SHELL_TAG);
			bzero(cmd, CMD_LEN);
	   }
	   else
	   {
	      strncat(cmd, &c, (size_t) 1);
	   }
	}
	printf("\n");
	return 0;
}


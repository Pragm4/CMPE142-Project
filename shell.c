#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef void (*sighandler_t)(int);
char c = '\0';
char* SHELL_TAG = "[SHELL v0.1]";

void handle_signal(int signo) //Handler for CTRL-C signal
{
	printf("\n%s ", SHELL_TAG);
	fflush(stdout); //flush output to stdout
}

int main(int argc, char *argv[], char *envp[])  //envp[] = array of environment vars
{
	//signal(SIGINT, SIG_IGN); //ignores SIGINT signal
	
	signal(SIGINT, handle_signal); //SIGINT = signal for CTRL-C
	printf("%s ", SHELL_TAG);
	while(c != EOF) {
		c = getchar();
		if(c == '\n')
			printf("%s ", SHELL_TAG);
	}
	printf("\n");
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

/*Definitions*/
#define CMD_LEN 100
#define ARGS_LEN 100 //Array of argument strings
#define ARG_LEN 50   //Single argument string
#define ENV_LEN 100

/*Type definition for signal handling*/
typedef void (*sighandler_t)(int);

/*Placeholders*/
char c = '\0';
char *SHELL_TAG = "[SHELL v0.1]";
char *shellArgs[ARGS_LEN];
char *shellEnv[ENV_LEN];

/*External Array tracking Environment Variables*/
extern char **environ;

void populateArgs(char *input)
{
   char c;
   char arg[ARG_LEN];
   char *temp = input;
   int i = 0;
   bzero(arg, ARG_LEN);
   while(*temp != '\0')
   {
      if(*temp == ' ')
      {
         if(shellArgs[i] == NULL)
         {
            shellArgs[i] = (char*) malloc( sizeof(char)* (strlen(arg) + 1) );
         }
         else
         {
            bzero(shellArgs[i], strlen(shellArgs[i]));
         }
         strncpy(shellArgs[i], arg, strlen(arg));
         strncat(shellArgs[i], "\0", 1);
         bzero(arg, ARG_LEN);
         i++;
      }
      else
      {
         strncat(arg, temp, 1);
      }
      temp++; //increment pointer to string of args (temp[i] = temp[i+1])
      /*
      //DEBUGGING PURPOSES: Display arg string by character
      printf("temp = %c\n", *temp);
      */
   }
   shellArgs[i] = (char*) malloc(sizeof(char)* (strlen(arg) + 1) );
   strncpy(shellArgs[i], arg, strlen(arg));
   strncat(shellArgs[i], "\0", 1);
}

void freeArgs()
{
   int i;
   for(i = 0; shellArgs[i] != NULL; i++)
   {
      bzero(shellArgs[i], strlen(shellArgs[i]));
      shellArgs[i] = NULL;
      free(shellArgs[i]);
   }
}

void execute(char *cmd)
{
	int i;
	if(fork() == 0) {
	
      /*UNDER CONSTRUCTION*/
      /*This command works, but takes no parameters or environment variables*/
		i = execve(cmd, shellArgs, environ);
		/*END OF CONSTRUCTION*/
		
		if(i < 0) //Command not found
		{
			printf("%s: %s\n", cmd, "*command not found*");
			exit(1);		
		}
	}
	else //Command found
	{
		wait(NULL);
	}
}

void handle_signal(int signo) //Handler for CTRL-C signal
{
	printf("\n%s ", SHELL_TAG);
	fflush(stdout); //flush output to stdout
}

int main()
{
   int i;
   char c;
   char *cmd = (char*)malloc(sizeof(char) * CMD_LEN);
   char *temp = (char*)malloc( sizeof(char) * (CMD_LEN+ARGS_LEN) );
   
	//signal(SIGINT, SIG_IGN); //ignores SIGINT signal
	signal(SIGINT, handle_signal); //SIGINT = signal for CTRL-C
	
	printf("%s ", SHELL_TAG);
	//Shell will be terminated when getchar() returns end-of-file (CTRL-D)
	while(c != EOF)
	{
		c = getchar();
		if(c == '\n')
		{
		   /*Populates shellArgs with parameters within temp*/
		   populateArgs(temp);
		   
		   /*places first argument into command placeholder*/
		   strcpy(cmd, shellArgs[0]);
		   
		   /*Call to fork and execlp with input command*/
		   execute(cmd);
			printf("%s ", SHELL_TAG);
			
			/*free and unset memory*/
			freeArgs();
			bzero(cmd, CMD_LEN);
			bzero(temp, CMD_LEN+ARGS_LEN);
	   }
	   else
	   {
	      strncat(temp, &c, (size_t) 1);
	   }
	}
	
	/*free up memory*/
	free(temp);
	free(cmd);
	for(i = 0; shellArgs[i] != NULL; i++) free(shellArgs[i]);
	for(i = 0; shellEnv[i] != NULL; i++) free(shellEnv[i]);
	
	printf("\n");
	return 0;
}


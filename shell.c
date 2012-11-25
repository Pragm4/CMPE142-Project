#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

/*Definitions*/
#define CMD_LEN 100
#define ARGS_LEN 100 //Array of argument strings
#define ARG_LEN 50   //Single argument string
#define ENV_LEN 100
#define PATHS_LEN 50

/*Type definition for signal handling*/
typedef void (*sighandler_t)(int);

/*Placeholders*/
char c = '\0';
char *SHELL_TAG = "[SHELL v0.1]";
char *shellArgs[ARGS_LEN];
char *shellEnv[ENV_LEN];
char *shellPaths[PATHS_LEN];

void initializeEnv(char **envp)
{
   int i = 0;
   while(envp[i] != NULL)
   {
      shellEnv[i] = (char*) malloc(sizeof(char) * (strlen(envp[i])+1));
      memcpy(shellEnv[i], envp[i], strlen(envp[i]));
      i++;  
   }
}

void initializePaths()
{
   char *p;
   char *q;
   int i, j;
   
   i = 0;
   while(shellEnv[i] != NULL)
   {
      p = strstr(shellEnv[i], "PATH");
      if(p != NULL && p - shellEnv[i] == 0)
      {
         j = 0;
         p = strstr(p, "=");
         while(p != NULL)
         {
            p++;
            q = strstr(p, ":");
            if(q == NULL)
            {
               shellPaths[j] = (char*) malloc(sizeof(char)*(strlen(p)+1));
               strncpy(shellPaths[j], p, strlen(p));
               strncat(shellPaths[j], "\0", 1);
            }
            else
            {
               shellPaths[j] = (char*) malloc(sizeof(char)*(q-p+1));
               strncpy(shellPaths[j], p, q-p);
               strncat(shellPaths[j], "\0", 1);
            }
            j++;
            p = strstr(p, ":");
         }
         break;      
      }
      i++;
   }
}

void pathPrepend(char *cmd)
{
   int fd;
   char temp[CMD_LEN];
   int i = 0;
   
   fd = open(cmd, O_RDONLY);
   if(fd > 0) return;
   
   while(shellPaths[i] != NULL)
   {      
      //executable = shellPath + '/' + command 
      strncpy(temp, shellPaths[i], strlen(shellPaths[i]));
      strncat(temp, "/", 1);
      strncat(temp, cmd, strlen(cmd));
      
      //Check if executable is valid      
      fd = open(temp, O_RDONLY);
      if(fd > 0)
      {
         close(fd);
         bzero(cmd, CMD_LEN);
         strncpy(cmd, temp, strlen(temp));
         return;
      }
      bzero(temp, CMD_LEN);
      i++;
   }
}

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
		i = execve(cmd, shellArgs, shellEnv);
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

int main(int argc, char **argv, char **envp)
{
   int i;
   char c;
   char *cmd = (char*)malloc(sizeof(char) * CMD_LEN);
   char *temp = (char*)malloc( sizeof(char) * (CMD_LEN+ARGS_LEN) );
   
	//signal(SIGINT, SIG_IGN); //ignores SIGINT signal
	signal(SIGINT, handle_signal); //SIGINT = signal for CTRL-C
	
	initializeEnv(envp);
	initializePaths();
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
		   
		   /*Finds and prepends full path to valid command*/
		   pathPrepend(cmd);
		   
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

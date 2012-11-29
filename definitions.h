#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


/*Definitions*/
#define MAX_SIZE 50
#define MAX_LEN 200
#define RESERVED " !@#$%^&*()-_=+1234567890,.<>{}\\|`~\"\'\0\1\2\3\4\5\6\7\a\b\f\n\r\t\v"

/*Type definition for signal handling*/
typedef void (*sighandler_t)(int);

/*Placeholders*/
char c = '\0';
char *SHELL_TAG = "[SHELL v0.1]";
char *shellArgs[MAX_SIZE];
char *shellEnv[MAX_SIZE];
char *shellPaths[MAX_SIZE];

/*Prototypes - Alphabetically*/
void execute(char *cmd);
void executeSeries(char *var, int floor, int ceil);
void freeArgs();
char *getEV(char *var);
void handle_signal(int signo);
void initializeEnv(char **envp);
void initializePaths();
void interprateEVs(char **cmd);
int parseShellCommands(char *cmd);
void pathPrepend(char *cmd);
void populateArgs(char *input);
void processCommand(char *temp);
void setEV_i(const char *var, int value); //Calls on setEV_s after converting value to string
void setEV_s(const char *var, char *value);

/*Functions ordered according to prototype list*/

void execute(char *cmd)
{
	int i;
	if(fork() == 0)
	{
		i = execve(cmd, shellArgs, shellEnv);
		
		if(i < 0) //Command not found
		{
			printf("%s: %s\n", cmd, "*command not found*");
			exit(1);		
		}
	}
	else
	{
		wait(NULL);
	}
}

void executeSeries(char *var, int floor, int ceil)
{
	char *commands[MAX_SIZE];
	char *temp;
	int i,j;
	
	for(i = 0; i < MAX_SIZE; i++) commands[i] = NULL;
	
	setEV_i(var, floor);

	//Input series of commands
	i=0;
	do
	{
		temp = (char*)malloc(sizeof(char)*MAX_LEN);
		bzero(temp, MAX_LEN);
		gets(temp);
		
		if(strlen(temp) == 0) continue;
		if(!strcmp(temp, "forend")) break;
		
		commands[i] = (char*)malloc(sizeof(char)*(strlen(temp)+1));
		strncpy(commands[i], temp, strlen(temp));
		strncat(commands[i], "\0", 1);
		
		bzero(temp, MAX_LEN);
		i++;
	}while(i < MAX_SIZE);

	for(i = atoi(getEV(var)); i < ceil; setEV_i(var, i))
	{
		j = 0;
		while(commands[j] != NULL)
		{
			bzero(temp, MAX_LEN);
			strcpy(temp, commands[j]);
			processCommand(temp);
			j++;
		}
		i++;
	}

	i = 0;
	while(commands[i] != NULL)
	{
		bzero(commands[i], strlen(commands[i]));
		commands[i] = NULL;
		free(commands[i]);
		i++;
	}
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

char *getEV(char *var)
{
	char *x;
	char *temp = (char*)malloc(sizeof(char) * (strlen(var)+2));
	strncpy(temp, var, strlen(var));
	strncat(temp, "=", 1);
	strncat(temp, "\0", 1);
	
	int i = 0;
	while(shellEnv[i] != NULL)
	{
		x = strstr(shellEnv[i], temp);
		if(x != NULL && x - shellEnv[i] == 0)
		{
			return strstr(shellEnv[i], "=")+1;
		}
		i++;
	}
	return "";
}

void handle_signal(int signo) //Handler for CTRL-C signal
{
	printf("\n%s ", SHELL_TAG);
	fflush(stdout); //flush output to stdout
}

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

void interprateEVs(char **cmd)
{
	int i, j, n;
	char *temp, *rv, *ev;
	
	ev = (char*)malloc(sizeof(char)*MAX_LEN);
	rv = (char*)malloc(sizeof(char)*MAX_LEN);
	temp = (char*)malloc(sizeof(char)*MAX_LEN);
	bzero(ev, MAX_LEN);
	bzero(rv, MAX_LEN);
	bzero(temp, strlen(*cmd));
	memcpy(temp, *cmd, MAX_LEN);
	
	while(*temp != '\0')
	{
		if(*temp == '$')
		{
			temp++;
			while(index(RESERVED, *temp) == NULL && *temp != '\0')
			{
				strncat(ev, temp, 1);
				temp++;
			}
			strcat(rv, getEV(ev));
			bzero(ev, strlen(ev));
		}
		else
		{
			strncat(rv, temp, 1);
			temp++;
		}
	}
	bzero(*cmd, MAX_LEN);
	memcpy(*cmd, rv, MAX_LEN);
}

int parseShellCommands(char *cmd)
{
	int i;
	char *iter, *floor, *ceil;
	char *temp;
	
	interprateEVs(&cmd);
	
	//Check EV assignment
	temp = index(cmd, '=');
	if(temp != NULL)
	{
		i = temp - cmd;
		if(i > 0)
		{
			temp++;
			cmd = cmd + i;
			strncpy(cmd, "\0", 1);
			cmd = cmd - i;
			setEV_s(cmd, temp);
		}
		else
		{
			printf("%s\n", "Cannot assign environment variable: Must supply a variable name.");
		}
		return 1;
	}
	
	//Check for For <var> <floor> <ceil> command
	temp = strstr(cmd, "For ");
	if(temp != NULL && temp - cmd == 0)
	{
		temp = temp + strlen("For ");
		i = index(temp, ' ') - temp;

		iter = (char*)malloc(sizeof(char)*(i+1));
		strncpy(iter, temp, i);
		strncat(iter, "\0", 1);

		temp = temp + i + 1;
		i = index(temp, ' ') - temp;

		floor = (char*)malloc(sizeof(char)*(i+1));
		strncpy(floor, temp, i);
		strncat(floor, "\0", 1);
		
		temp = temp + i + 1;
		
		ceil = (char*)malloc(sizeof(char)*(strlen(temp)+1));
		strncpy(ceil, temp, strlen(temp));
		strncat(ceil, "\0", 1);
		
		executeSeries(iter, atoi(floor), atoi(ceil));
		return 1;
	}
	return 0;
}

void pathPrepend(char *cmd)
{
   int fd;
   char temp[MAX_LEN];
   int i = 0;
   
   fd = open(cmd, O_RDONLY);
   if(fd > 0)
   {
   	close(fd);
   	return;
   }
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
         bzero(cmd, MAX_LEN);
         strncpy(cmd, temp, strlen(temp));
         return;
      }
      bzero(temp, MAX_LEN);
      i++;
   }
}

void populateArgs(char *input)
{
   char c;
   char arg[MAX_LEN];
   char *temp = input;
   int i = 0;
   bzero(arg, MAX_LEN);
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
         bzero(arg, MAX_LEN);
         i++;
      }
      else
      {
         strncat(arg, temp, 1);
      }
      temp++;
   }
   shellArgs[i] = (char*) malloc(sizeof(char)* (strlen(arg) + 1) );
   strncpy(shellArgs[i], arg, strlen(arg));
   strncat(shellArgs[i], "\0", 1);
}

void processCommand(char *temp)
{
   char cmd[MAX_LEN];
   int i;
   i = parseShellCommands(temp);
   if(i == 0)
   {
		populateArgs(temp);
		strcpy(cmd, shellArgs[0]);
		pathPrepend(cmd);
		execute(cmd);
	   freeArgs();
	}
}

void setEV_i(const char *var, int value)
{
	char *tempValue;
	tempValue = (char*)malloc(sizeof(char)*10);
	sprintf(tempValue, "%d", value);
	setEV_s(var, tempValue);
}

void setEV_s(const char *var, char *value)
{
	char *temp;
	int i = 0;
	while(shellEnv[i] != NULL)
	{
		temp = strstr(shellEnv[i], var);
		if(temp != NULL && temp - shellEnv[i] == 0)
		{
			bzero(shellEnv[i], strlen(shellEnv[i]));
			shellEnv[i] = NULL;
			free(shellEnv[i]);
			shellEnv[i] = (char*)malloc(sizeof(char) * (strlen(var)+strlen(value)+2));
			strncpy(shellEnv[i], var, strlen(var));
			strncat(shellEnv[i], "=", 1);
			strncat(shellEnv[i], value, strlen(value));
			strncat(shellEnv[i], "\0", 1);
			return;
		}
		i++;
	}
	shellEnv[i] = (char*)malloc(sizeof(char) * (strlen(var)+strlen(value)+2));
	strncpy(shellEnv[i], var, strlen(var));
	strncat(shellEnv[i], "=", 1);
	strncat(shellEnv[i], value, strlen(value));
	strncat(shellEnv[i], "\0", 1);
}

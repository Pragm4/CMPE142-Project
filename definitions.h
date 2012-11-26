/*Definitions*/
#define MAX_SIZE 50
#define MAX_LEN 100

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
void freeArgs();
char* getEV(char *var);
void handle_signal(int signo);
void initializeEnv(char **envp);
void initializePaths();
void pathPrepend(char *cmd);
void populateArgs(char *input);
void processCommand(char *temp);
void setEV(const char *var, char *value);

/*Functions ordered according to prototype list*/

void execute(char *cmd)
{
	int i;
	if(fork() == 0) {

		i = execve(cmd, shellArgs, shellEnv);
		
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
	return NULL;
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

void pathPrepend(char *cmd)
{
   int fd;
   char temp[MAX_LEN];
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
         bzero(cmd, MAX_LEN);
         strncpy(cmd, temp, strlen(temp));
         return;
      }
      bzero(temp, MAX_LEN);
      i++;
   }
   //bzero(cmd, MAX_LEN);
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

void processCommand(char *temp)
{
   char cmd[MAX_LEN];
   populateArgs(temp);
   strcpy(cmd, shellArgs[0]);
   pathPrepend(cmd);
   execute(cmd);
}

void setEV(const char *var, char *value)
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

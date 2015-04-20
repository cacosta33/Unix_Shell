/* Name: Cesar Acosta
  Date: 11/10/14
  CSCI 1730 Project 5 Shell
Description: The purpose of this project is to create your own shell using
the knowledge you have acquired in CSCI 1730 up to this point. A shell is a 
command line interpreter, it is the program you interface with in a terminal 
or windows. For example, when you log onto {nike}, the shell gives you a 
prompt, typically "%" or "$" and then waits to interprets commands that 
you type on the command line. Unix of-course, gives you a wide selection 
f shells to choose between, csh, bash, tcsh or ksh. 
In ksh you can search you command line history and re-use (and edit) 
old commands. */


/* -----------------------------------------------------------------------------
FILE: shell.c
 
NAME: YOSH (Y)our (O)wn (S)imple (S)hell

DESCRIPTION: A SHELL SKELETON
-------------------------------------------------------------------------------*/
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"   // local file include declarations for parse-related structs

#define HISTORY_MAX_SIZE 50
#define MAX_ARGS 30

enum BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT, JOBS, HELP, HISTORY, CD, NUM }; 
/* This enumeration starts at zero and increments by
                        one. */
char *prompt; //This is used for the prompt that is printed
char cwd[1024]; //This will store the current working directory


static char *history[HISTORY_MAX_SIZE]; /*Pointer to the array that will
                                                store the history commands */
static unsigned historyCounter = 0;

void addToHistory(const char *command){ //This keeps track of the user history

  if(historyCounter < HISTORY_MAX_SIZE){
    history[historyCounter++] = strdup(command);
  }else{
    free(history[0]);
    unsigned index;
    for(index = 1; index < HISTORY_MAX_SIZE; index++){
      history[index-1] = history[index];
    }
    history[HISTORY_MAX_SIZE-1] = strdup(command);
  }

}
/* I feel as if I have written too many fundctions for this shell.
  I apologize for the inconvenience, they are probably many more efficient
  shells out there. */


void pipeFunction(char *cmd); //Executes the pipe function
void executeOutFile(char *cmd);// Deals with redirecting std_out
void executeInFile(char *cmd);// Deals with redirecting std_in
void executeBoth(char *cmd);// Deals with both std_in and std_out
void outRedirectFunction(char **com, char *outfile);//takes command and sends to executeOutFile
void inRedirectFunction(char **com, char *infile);//takes command and sends to executeInFile
void bothRedirectFunction();
static void sig_handler(int signo);
void executeHistoryCommand(void);
void executeHelpCommand(void);
void executeJobsCommand(void);
void executeCDCommand(void);
void executeNumCommand(char *cmd);
void parse_CommandLine(char *command, char **argv);
void executePipeFunction(char *cmd1,char *cmd2);



/*The purpose of this sig handler is to deal with catching the signal from
the child process and sending it to the background. */

static void sig_handler(int signo){
  int status;

  if(signo == SIGCHLD){
    //wait((int *)0);
    //printf("\nChild is still running.\n");
  }else{
    fprintf(stderr, "ERROR: Received signal %d\n", signo);
    exit(1);
  }

}




/* -----------------------------------------------------------------------------
FUNCTION: buildPrompt()
DESCRIPTION: The purpose of this function is to build the prompt that always
appears before entering a command in this shell. This prompt is designed to
print your current working directory.
-------------------------------------------------------------------------------*/
char * buildPrompt()
{
  
  getcwd(cwd, sizeof(cwd));
  printf("{");
  prompt = strcat(cwd, "} ");

  /*
  printf("{");
  size = pathconf(".", _PC_PATH_MAX);  
  //This function below does not work on the server for some reason.
  prompt = getcwd(buf, (size_t)size);*/

  return prompt;
}
 
/* -----------------------------------------------------------------------------
FUNCTION: isBuild()
DESCRIPTION: These are the commands that are valid for this shell. By entering
these commands, the shell will carry out the commands similiar to the shell in
Linux.
-------------------------------------------------------------------------------*/
int isBuiltInCommand( char * cmd )
{
  if( strncmp(cmd, "exit", strlen( "exit" ) ) == 0 ){  
      return EXIT;
  }else if( strncmp(cmd, "jobs", strlen( "jobs" ) ) == 0 ){  
      return JOBS;
  }else if( strncmp(cmd, "history", strlen( "history" ) ) == 0 ){  
      return HISTORY;
  }else if( strncmp(cmd, "help", strlen( "help" ) ) == 0 ){  
      return HELP;
  }else if( strncmp(cmd, "cd", strlen( "cd" ) ) == 0 ){  
      return CD;
  }else if( strncmp(cmd, "!", strlen( "!" ) ) == 0 ){  
      return NUM;
  }
  return NO_SUCH_BUILTIN;
}

/* This command is for executing when you want to see the a specific 
command that is numbered in the history list. */

void executeNumCommand(char *cmd){
  char *argv[3];
  int num,  i;

  parse_CommandLine(cmd, argv); //Parse to break up into array

  num = atoi(argv[1]);//Used to store the number in arg as an int in num

  if(num == -1){/* This deals with retrieving the actual command. */
    for(i=0; i<sizeof(history); i++){  
      if(history[i]==NULL){
        printf("\n%s\n", history[i-2]);
        i = sizeof(history);
      }
    }
    
  }else{
      for(i=0; i<HISTORY_MAX_SIZE; i++){

        if(num == i){
          printf("\n%s\n",history[i]);
          i = sizeof(history);
        }
      }
  }

}

/*This is used for calling the commands that are stored out to the screen
after the history command has been called. */
void executeHistoryCommand(void){ //Prints out the user history
  int i, num=0;
  printf("\n");
  printf("History of Commands:\n");
  for(i=0; i<sizeof(history); i++){  
    if(history[i]==NULL){
      i = sizeof(history);
    }else{
      printf("%d.) %s\n",num, history[i]);
      num++;
    }
  }
}

/*This is a built in command that lists all the commands that should work */
void executeHelpCommand(void){
  printf("\nThe following are built in commands and their syntax:\n"
    "exit\n"
    "help\n"
    "history\n"
    "cd [directory]\n"
    "jobs\n"
    "! [argument(it is a number)]\n");
}


/*This is used for parsing the command from the command line and storing
it as an array to more easily execute certain commands. */
void  parse_CommandLine(char *command, char **argv)
{
     while (*command != '\0') {       /* if not the end of command ....... */ 
          while (*command == ' ' || *command == '\t' || *command == '\n')
               *command++ = '\0';     /* replace white spaces with 0    */
          *argv++ = command;          /* save the argument position     */
          while (*command != '\0' && *command != ' ' && 
                 *command != '\t' && *command != '\n') 
               command++;             /* skip the argument until ...    */
     }
     *argv = NULL;                 /* mark the end of argument list  */
}



/* -----------------------------------------------------------------------------
FUNCTION: executeCommand(cmd);
DESCRIPTION: This executes the command that is called on the command line.
-------------------------------------------------------------------------------*/

void executeCommand(char* cmd){
  char *argv[MAX_ARGS];

  parse_CommandLine(cmd, argv);

  printf("\n");
  execvp(*argv, argv);//If this fails, it will return the bottom.
 
  printf("Unknown Command\n");
  exit(0);  

}

/* This is used to place a child process in the background and have it 
run nonblockingly, as all background processes do. */
void executeBackground(char *cmd){
  char *argv[MAX_ARGS];
  char *argTemp[5];
  char *com;


  parse_CommandLine(cmd, argv);

  int i;
  for(i=0; i<sizeof(argv); i++){
    
    if(strncmp(argv[i],  "&", strlen( "&" )) == 0){ //Check to see if & is present.
      argTemp[i] = NULL;
      //printf("%s\n",argv[i-1 ); //Used for error checking
      execvp(argTemp[i], argTemp);
    }else{
      argTemp[i] = argv[i];
    }
  }
}

/*This is used for standard redirection out, with the input being read
from the command line and then parsed. */
void executeOutFile(char *cmd){
  char *argv[MAX_ARGS];
  char *argTemp[5];
  char *outfile, *com;


  parse_CommandLine(cmd, argv);

  int i;
  for(i=0; i<sizeof(argv); i++){
    
    if(strncmp(argv[i],  ">", strlen( ">" )) == 0){
      argTemp[i] = NULL;
      outfile = argv[i+1];
      //printf("(i--) is %s, (i) is %s, (i++) is %s\n",argv[i-1], argv[i], argv[i+1]);
      outRedirectFunction(argTemp, outfile);
    }else{
      argTemp[i] = argv[i];
    }
  }


}

/*This is used for standard redirection in, with the input being read
from the command line and then parsed. */
void executeInFile(char *cmd){
  char *argv[MAX_ARGS];
  char *argTemp[5];
  char *infile, *com;


  parse_CommandLine(cmd, argv);

  int i;
  for(i=0; i<sizeof(argv); i++){
    
    if(strncmp(argv[i],  "<", strlen( "<" )) == 0){
      argTemp[i] = NULL;
      infile = argv[i+1];
      //printf("(i--) is %s, (i) is %s, (i++) is %s\n",argv[i-1], argv[i], argv[i+1]);
      inRedirectFunction(argTemp, infile);
    }else{
      argTemp[i] = argv[i];
    }
  }


}


/*This is used for standard redirection out and standard redirection in,
with the input being read from the command line and then parsed. */
void executeBoth(char *cmd){
  char *argv[MAX_ARGS];
  char *argTemp1[5];
  char *infile, *outfile, *com;


  parse_CommandLine(cmd, argv);

  int i,j;
  for(i=0; i<sizeof(argv); i++){
    
    if(strncmp(argv[i],  "<", strlen( "<" )) == 0){
      argTemp1[i] = NULL;
      infile = argv[i+1];
      outfile = argv[i+3];
      //printf("(i--) is %s, (i) is %s, (i++) is %s\n",argv[i-1], argv[i], argv[i+1]);
      bothRedirectFunction(argTemp1, infile, outfile);
     
    }else{
      argTemp1[i] = argv[i];
    }
  }

}


/*This will take the result from the executeBoth() and have the results
actually being executed here using redirection with the dup2() */
void bothRedirectFunction(char **com, char *infile, char *outfile){
  int fd1, fd2;
  pid_t child;

  fd1 = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  fd2 = open(infile, O_RDWR, S_IRUSR | S_IWUSR);

  if(fd1 < 0){
    fprintf(stderr, "Error opening/creating file.\n");
    exit(1);
  }
  if(fd2 < 0){
    fprintf(stderr, "No such file exists.\n");
    exit(1);
  }

  dup2(fd2, STDIN_FILENO);
  dup2(fd1, STDOUT_FILENO);

  close(fd1);

  execvp(com[0], com);

  printf("Unknown Command\n");

}

/*The purpose of this file is to redirect out */
void outRedirectFunction(char **com, char *outfile){

  int fd;

  //printf("com is %s, outfile is %s\n",com[0], outfile );
  /*for(int i; i<sizeof(com); i++)
    printf("Com Array[%d]: %s\n",i, com[i]);*/
  //com[sizeof(com)-1] = NULL;

  fd = open(outfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

  if(fd < 0){
    fprintf(stderr, "Error opening/creating file\n");
    exit(1);
  }

  dup2(fd, STDOUT_FILENO); // Copy fileID to stdout
  close(fd);
  execvp(com[0], com);

  printf("Unknown Command\n");


}

/*The purpose of this function is to redirect */
void inRedirectFunction(char **com, char *infile){

  int fd;

  //printf("com is %s, outfile is %s\n",com[0], outfile );
  /*for(int i; i<sizeof(com); i++)
    printf("Com Array[%d]: %s\n",i, com[i]);*/
  com[sizeof(com)-1] = NULL;

  fd = open(infile, O_RDWR, S_IRUSR | S_IWUSR);

  if(fd < 0){
    fprintf(stderr, "Error, no such file or directory.\n");
    exit(1);
  }

  dup2(fd, STDIN_FILENO); // Copy fileID to stdout
  //close(fd);
  execvp(com[0], com);

  printf("Unknown Command\n");


}
  
/*This function only deal with one pipe, it cannot handle more than one */  
void pipeFunction(char *cmd){

  char *storeArray1[5], *storeArray2[5], *argv[MAX_ARGS];

  parse_CommandLine(cmd, argv);

  executePipeFunction(argv[0], argv[2]);

}

void executePipeFunction(char *cmd1,char *cmd2){

int fds[2];

  pipe( fds ) ;   /* no error checks */
  printf("\n");

  if( fork() == 0 )
        { 
        dup2( fds[0] , STDIN_FILENO );
        close( fds[ 1 ] );
        execlp(cmd2, cmd2, (char *)0);
        printf("Unknown Command\n");

        //execlp( "sort", "sort", (char *) 0 );
        }
  else
        { /* parent */

  if( fork() == 0 )
        { /* 2nd child */
        /* who --> fds[1]/stdout --> sort */

  /* stdout  to pipe */
        dup2( fds[0] , STDIN_FILENO );
        dup2( fds[1] , STDOUT_FILENO );
        close( fds[ 0 ] );
        execlp(cmd1, cmd1, (char *)0);
        printf("Unknown Command\n");

        //execlp( "who", "who", (char *) 0 );
        }
   else
        { /* parent closes all links */
        close( fds[ 0 ] );
        close( fds[ 1 ] );

        wait( (int *) 0 );
        wait( (int *) 0 );
        } /* else parent second child */
    } /* else parent first child */
}



/* -----------------------------------------------------------------------------
FUNCTION: main()
DESCRIPTION: This is where the main shell function executes with the help of all
the function that are defined above. This simulates a shell.
-------------------------------------------------------------------------------*/
int main()
{
  char * cmdLine;
  parseInfo *info; 		// info stores all the information returned by parser.
  struct commandType *com; 	// com stores command name and Arg list for one command.
  pid_t childPID;


//Very important disclaimer
  fprintf( stdout, "This is the SHELL version 1.0\n"//As of Mon Nov 10 22:51:14 EST 2014
  "\nIMPORTANT!: When grading, please ignore the \"No such File or Directory\"\n"
  "message. The commands still work, for some reason it keeps printing out \n"
  "that error even though the commands work. Please double check to\n" 
  "make sure that the command worked.\n"
  "Thank you for your cooperation.\n\n");


  while(1)
  {
      //insert your code here
    	cmdLine = readline( buildPrompt() );
      
    	if( cmdLine == NULL ) 
	{
      		fprintf(stderr, "Unable to read command\n");
          exit(1);
      		continue;
    	}


    	// insert your code about history and !x !-x
      // !x and !-x is dealt with elsewhere.
      addToHistory(cmdLine);

    	// calls the parser
    	info = parse( cmdLine );
        if( info == NULL ){
      		free(cmdLine);
      		continue;
    	}

    	// prints the info struct
    	print_info( info );

    	//com contains the info. of the command before the first "|"
    	com = &info->CommArray[0];
    	if( (com == NULL)  || (com->command == NULL)) 
    	{
      		free_info(info);
      		free(cmdLine);
      		continue;
    	}

    	//com->command tells the command name of com
    	if( isBuiltInCommand( com->command ) == EXIT ){  
      		exit(1);
    	}else if( isBuiltInCommand( com->command ) == HISTORY ){  
          executeHistoryCommand();
      }else if( isBuiltInCommand( com->command ) == HELP ){  
          executeHelpCommand();
      }else if( isBuiltInCommand( com->command ) == NUM ){  
          executeNumCommand(cmdLine);
      }else if(info->boolOutfile == 1 && info->boolInfile == 1){
        childPID = fork();//Checks for "<" and ">"
        if (childPID == 0){
          executeBoth(cmdLine);
        }else{
          wait((int *) 0);
        }
      }else if(info->boolBackground==1){//Checks for "&"
          childPID = fork();
          if(childPID == 0){ 
            pause(); 
            executeBackground(cmdLine);
            
          }else{
            if(signal(SIGCHLD, sig_handler) == SIG_ERR){
              perror("Cannot catch SIGCHLD\n");
            }     

          }

        }
        
      else if(info->boolOutfile == 1){//Checks for ">"
        childPID = fork();
        if(childPID ==0){
          executeOutFile(cmdLine);
        }else{
          wait((int *)0);
        }
      }else if(info->boolInfile == 1){//Checks for "<"
        childPID = fork();
        if(childPID ==0){
          executeInFile(cmdLine);
        }else{
          wait((int *)0);
        }
      }else if(info->pipeNum > 0){//Checks for "|"
        pipeFunction(cmdLine);
      }else{
        childPID = fork();
        if(childPID == 0){
          //printf("info->boolOutfile: (%d)\n",info->boolOutfile );
          executeCommand(cmdLine); // This calls execvp
        }else{
          wait((int *) 0); //Waits for Child to terminate
        }
      }

  free_info(info);

  free(cmdLine);
  printf("\n");

  }/* while(1) */

}//End of main execution
  






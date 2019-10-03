#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "job.h"

#define CMD_MAX 512

void display_prompt();
void read_command(char ***command);

int main(int argc, char *argv[])
{

  while (1) //repeat forever 
  {
    int status;
    //char **command;
    
    display_prompt();                 // Display prompt in terminal     
    //read_command(&command);           // Read input from terminal 
    
    //TESTING
    char *input;
    input = (char *)malloc(CMD_MAX * sizeof(char));
    size_t size = CMD_MAX;
    int num = getline(&input, &size, stdin);
    
    num--;
    char *text = (char *)malloc(num * sizeof(char));
    for (int i = 0; i < num; i++) 
    {
      if (input[i] == '\n')
        break;
      text[i] = input[i];
    }
    
    printf("%d", (int)strlen(text));
    char *command[2] = {text, NULL};
    //FOR TESTING

    if (fork() != 0)
    {                                 // fork off child process  Parent       
      waitpid(-1, &status, 0);        // wait for child to exit  
      fprintf(stderr, "+ completed %s [%d]\n", command[0], status);
    }
    else
    {
      execvp(command[0], command);     // execute command
      perror("execvp");                // coming back here is an error 
      exit(1);
    }
  }
}

void display_prompt()
{
  printf("sshell$ ");
}

void read_command(char ***command)
{
  char *input;
  input = (char *)malloc(CMD_MAX * sizeof(char));
  size_t size = CMD_MAX;
  int num = getline(&input, &size, stdin); 
    
  num--;
  char *text = (char *)malloc(num * sizeof(char));
    
  for (int i = 0; i < num; i++) // this loop copies the command line to a string
  {
    if (input[i] == '\n')
      break;
    text[i] = input[i];
  }
    
  //**command[2] = {text, NULL};

  //*command = malloc(sizeof(char*) * 16);
  //for (int i = 0; i < 16; i++)
  //{
    //*command[i] = malloc(sizeof(char) * 16);
  //}
  
  // FREE ALL MALLOCS

}

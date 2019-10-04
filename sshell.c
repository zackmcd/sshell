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
void read_command(job **jobs);

int main(int argc, char *argv[])
{

  while (1) //repeat forever 
  {
    int status;
    //char **command;
    //job *jobs; 
    display_prompt();                 // Display prompt in terminal     
    //read_command(&jobs);           // Read input from terminal 
    
    //TESTING
    char *input;
    input = (char *)malloc(CMD_MAX * sizeof(char));
    size_t size = CMD_MAX;
    int num = getline(&input, &size, stdin);
    input[num-1] = '\0';
    
    char *command[2] = {input, NULL};
    //FOR TESTING

    if (fork() != 0)
    {                                 // fork off child process  Parent       
      waitpid(-1, &status, 0);        // wait for child to exit  
      fprintf(stderr, "+ completed %s [%d]\n", command[0], status);
    }
    else
    {
      execvp(command[0], command);     // execute command
      //execvp(jobs->exec, jobs->args);
      perror("execvp");                // coming back here is an error 
      exit(1);
    }
  }
}

void display_prompt()
{
  printf("sshell$ ");
}

void read_command(job **jobs)
{
  char *input;
  input = (char *)malloc(CMD_MAX * sizeof(char));
  size_t size = CMD_MAX;
  int num = getline(&input, &size, stdin); 
  input[num - 1] = '\0'; 
  
  //int beg = 0;
  /*int end = 0;
  bool check = true; // checks to see if there are any other args than just the command

  for (int i = 0; i <= strlen(input); i++)
  {
    if (input[i] == '\0' && check)
    {
      char *line[2] = {input, NULL};
      job_create(input, line);
    }
    else if (isspace(input[i]))
    {
      check = false;
    }
    else
    {
      end++;
    }
  }*/

  //**command[2] = {input, NULL};

  //*command = malloc(sizeof(char*) * 16);
  //for (int i = 0; i < 16; i++)
  //{
    //*command[i] = malloc(sizeof(char) * 16);
  //}
  
  // FREE ALL MALLOCS

}

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
    
    job **jobs = malloc(5 * sizeof(job*));
    jobs[0] = job_create(); 
    display_prompt();                 // Display prompt in terminal     
    read_command(jobs);    //&jobs       // Read input from terminal 
    
    /* TESTING
    printf("wow2 %s\n", jobs[0]->exec);
    for (int i = 0; i < 16; i++)
    {
      if (jobs[0]->args[i] != NULL)
        printf("wow3 %s\n", jobs[0]->args[i]);
      else
        printf("oh boy\n"); 
    }
    */ //TESTING

    if (fork() != 0)
    {                                 // fork off child process  Parent       
      waitpid(-1, &status, 0);        // wait for child to exit  
      fprintf(stderr, "+ completed %s [%d]\n", jobs[0]->exec, status);
    }
    else
    {
      execvp(jobs[0]->exec, jobs[0]->args);
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
  
  int beg = 0;
  int end = 0;
  bool check = true; // checks to see if there are any other args than just the command
  int numJobs = 0;

  for (int i = 0; i <= strlen(input); i++)
  {
    if (input[i] == '\0' && check)
    {
      job_setExec(jobs[numJobs], input);
      job_addArg(jobs[numJobs], input);
    }
    else if (isspace(input[i]) || input[i] == '\0')
    {
      if (input[i] == '\0')
        end++;
      
      check = false;
      int index = end - beg;
      char word[index];
      
      for (int j = 0; j < index; j++)
      {
        word[j] = input[beg];
	beg++;
      }

      word[index] = '\0';
      
      if (jobs[numJobs]->exec == NULL)
      {
        job_setExec(jobs[numJobs], word);
	job_addArg(jobs[numJobs], word);
      }
      else
      {
        job_addArg(jobs[numJobs], word);
      }

      beg++;
    }
    else if (input[i] == '|')
    {
      numJobs++;
    }
    else
    {
      end++;
    }
  }
  
  // FREE ALL MALLOCS

}

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "job.h"

#define CMD_MAX 512

void display_prompt();
void read_command(job **jobs, int *numJobs);
void ExecWithRedirector(job *cmd);

int main(int argc, char *argv[])
{
  while (1) //repeat forever
  {
    int status;

    job **jobs = malloc(5 * sizeof(job *));
    int numJobs = 0;
    jobs[numJobs] = job_create();
    display_prompt();             // Display prompt in terminal
    read_command(jobs, &numJobs); //&jobs       // Read input from terminal

    //TESTING
    printf("cmd1 is %s, infile is %s, outfile is %s\n", jobs[0]->exec, jobs[0]->infile, jobs[0]->outfile);

    for (int i = 0; i < 16; i++)
    {
      if (jobs[0]->args[i] != NULL)
        printf("arg%d is %s\n", i, jobs[0]->args[i]);
    }
    // printf("cmd2 is %s, infile is %s, outfile is %s\n", jobs[1]->exec,jobs[1]->infile,jobs[1]->outfile);
    // for (int i = 0; i < 16; i++)
    // {
    //   if (jobs[1]->args[i] != NULL)
    //     printf("cmd2 %s\n", jobs[1]->args[i]);
    // }

    //TESTING
    //printf("N%d\n", numJobs);
    if (jobs[0]->error)
    {
      for (int i = 0; i <= numJobs; i++)
        free(jobs[i]);
      free(jobs);
      continue;
    }

    /*if (numJobs > 0)
    {
      
    }
    else
    {*/

    if (fork() != 0)
    {                                                                  // fork off child process  Parent
      waitpid(-1, &status, 0);                                         // wait for child to exit
      fprintf(stderr, "+ completed %s [%d]\n", jobs[0]->line, status); // must print the entire command
    }
    else
    {
      //execvp(jobs[0]->exec, jobs[0]->args);
      ExecWithRedirector(jobs[0]);
      //perror("execvp");                // coming back here is an error
      exit(1);
    }

    //}

    //FREE mallocs
    for (int i = 0; i <= numJobs; i++)
      free(jobs[i]);
    free(jobs);
  }
}

void display_prompt()
{
  printf("sshell$ ");
}

void read_command(job **jobs, int *numJobs)
{
  char *input;
  input = (char *)malloc(CMD_MAX * sizeof(char));
  size_t size = CMD_MAX;
  int num = getline(&input, &size, stdin);
  input[num - 1] = '\0';

  int beg = 0;
  int end = 0;
  bool check = true; // checks to see if there are any other args than just the command
  //int numJobs = 0;

  job_setLine(jobs[*numJobs], input);

  for (int i = 0; i <= strlen(input); i++)
  {
    if (input[i] == '\0' && check)
    {
      job_setExec(jobs[*numJobs], input);
      job_addArg(jobs[*numJobs], input);
    }
    else if (isspace(input[i]) || input[i] == '\0' || input[i] == '|' || input[i] == '<' || input[i] == '>')
    {
      // printf("%d - %d = %d\n", end, beg, end - beg);
      if ((isspace(input[i - 1]) && isspace(input[i]))) // to deal with multiple spaces in command
      {
        beg++;
        end++;
        continue;
      }

      if (input[i] == '\0') // to handle the null terminator at the end of the command
        end++;

      check = false;
      int index = end - beg;
      char word[index];

      for (int j = 0; j < index; j++) // copies argument from command
      {
        word[j] = input[beg];
        beg++;
      }

      word[index] = '\0'; // adds null terminator in string
      if (index != 0)
      {

        if (jobs[*numJobs]->exec == NULL) // if it is the command
        {
          //printf("A%d = %s\n", *numJobs, word);
          job_setExec(jobs[*numJobs], word);
          job_addArg(jobs[*numJobs], word);
        }
        else if (jobs[*numJobs]->input) // last word is in-redirection
        {
          job_setInFile(jobs[*numJobs], word);
          job_setIn(jobs[*numJobs], false);
        }
        else if (jobs[*numJobs]->output) // last word is out-redirection
        {
          job_setOutFile(jobs[*numJobs], word);
          job_setOut(jobs[*numJobs], false);
        }
        else //if it is an argument
        {
          //printf("A%d = %s\n", *numJobs, word);
          job_addArg(jobs[*numJobs], word);
        }
      }

      if (input[i] == '|')
      {
        *numJobs = *numJobs + 1;
        jobs[*numJobs] = job_create();
      }
      if (input[i] == '<')
      {
        job_setIn(jobs[*numJobs], true);
      }
      if (input[i] == '>')
      {
        job_setOut(jobs[*numJobs], true);
      }

      end++;
      beg++;
    }
    else // when its just a character in an argument
    {
      //printf("%d\n", i);
      end++;
    }
  }
  // FREE ALL MALLOCS
  free(input);
}

void ExecWithRedirector(job *cmd)
{
  if (cmd->infile)
  {
    int fd = open(cmd->infile, O_RDWR);
    printf("yes we opened infile, fd is %d\n", fd);
    dup2(fd, STDIN_FILENO);
    close(fd);
  }
  if (cmd->outfile)
  {
    int fd = open(cmd->outfile, O_RDWR);
    dup2(fd, STDOUT_FILENO);
    close(fd);
  }
  execvp(cmd->exec, cmd->args);
}
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
void read_command(job *cmd0);
void ExecWithRedirector(job *cmd);
void ExcecWithPipe(job *process1);

int main(int argc, char *argv[])
{
  while (1) //repeat forever
  {
    int status;

    // job **jobs = malloc(5 * sizeof(job *));
    int numJobs = 0;
    //jobs[numJobs] = job_create();
    job *cmd0 = job_create();
    display_prompt(); // Display prompt in terminal
    //read_command(jobs, &numJobs); //&jobs       // Read input from terminal
    read_command(cmd0);

    //TESTING
    printf("cmd1 is %s, infile is %s, outfile is %s\n", cmd0->exec, cmd0->infile, cmd0->outfile);

    for (int i = 0; i < 16; i++)
    {
      if (cmd0->args[i] != NULL)
        printf("arg%d is %s\n", i, cmd0->args[i]);
    }
    // printf("cmd2 is %s, infile is %s, outfile is %s\n", jobs[1]->exec,jobs[1]->infile,jobs[1]->outfile);
    // for (int i = 0; i < 16; i++)
    // {
    //   if (jobs[1]->args[i] != NULL)
    //     printf("cmd2 %s\n", jobs[1]->args[i]);
    // }

    //TESTING
    //printf("N%d\n", numJobs);
    if (cmd0->error)
    {
      for (int i = 0; i <= numJobs; i++)
        free(cmd0);
      //free(jobs);
      continue;
    }

    /*if (numJobs > 0)
    {
      
    }
    else
    {*/

    if (fork() != 0)
    {                                                               // fork off child process  Parent
      waitpid(-1, &status, 0);                                      // wait for child to exit
      fprintf(stderr, "+ completed %s [%d]\n", cmd0->line, status); // must print the entire command
    }
    else
    {
      //execvp(jobs[0]->exec, jobs[0]->args);
      // ExecWithRedirector(jobs[0]);
      ExcecWithPipe(cmd0);
      //perror("execvp");                // coming back here is an error
      exit(-1);
    }

    //}

    //FREE mallocs
    //   for (int i = 0; i <= numJobs; i++)
    //     free(jobs[i]);
    //   free(jobs);
    // }
  }
}

  void display_prompt()
  {
    printf("sshell$ ");
  }

  void read_command(job * job0)
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
    job *currentJob = job0;

    job_setLine(job0, input);

    for (int i = 0; i <= strlen(input); i++)
    {
      if (input[i] == '\0' && check)
      {
        job_setExec(currentJob, input);
        job_addArg(currentJob, input);
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

          if (currentJob->exec == NULL) // if it is the command
          {
            //printf("A%d = %s\n", *numJobs, word);
            job_setExec(currentJob, word);
            job_addArg(currentJob, word);
          }
          else if (currentJob->input) // last word is in-redirection
          {
            job_setInFile(currentJob, word);
            job_setIn(currentJob, false);
          }
          else if (currentJob->output) // last word is out-redirection
          {
            job_setOutFile(currentJob, word);
            job_setOut(currentJob, false);
          }
          else //if it is an argument
          {
            //printf("A%d = %s\n", *numJobs, word);
            job_addArg(currentJob, word);
          }
        }

        if (input[i] == '|')
        {
          //*numJobs = *numJobs + 1;
          currentJob->nextcmd = job_create();
          currentJob = currentJob->nextcmd;
        }
        if (input[i] == '<')
        {
          job_setIn(currentJob, true);
        }
        if (input[i] == '>')
        {
          job_setOut(currentJob, true);
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

  void ExecWithRedirector(job * cmd)
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

  void ExcecWithPipe(job * process1)
  {
    if (process1->nextcmd == NULL)
    {
      ExecWithRedirector(process1);
      exit(-1);
    }
    job *process2 = process1->nextcmd;

    int fd[2];
    pipe(fd); /* Create pipe */
    if (fork() == 0)
    {                             /* Child*/
      close(fd[0]);               /* Don't need read access to pipe */
      dup2(fd[1], STDOUT_FILENO); /* Replace stdout with the pipe */
      close(fd[1]);               /* Close now unused file descriptor */
      ExecWithRedirector(process1);
      exit(-1);
    }
    else
    {                      /* Parent*/
      close(fd[1]);        /* Don't need write access to pipe */
      dup2(fd[0],STDIN_FILENO);          /* And replace it with the pipe */
      close(fd[0]);        /* Close now unused file descriptor */
      if (process2->nextcmd != NULL)
        ExcecWithPipe(process2); //###### RECURSIVE HERE #####
      else
        ExecWithRedirector(process2);
    }
  }

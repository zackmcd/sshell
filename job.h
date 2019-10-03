#ifndef JOB_H
#define JOB_H

#define MAX_ARGS 16

typedef struct {
  char *exec;
  char **args;
  char *file;
  bool input;
  bool output;
} job;

job* job_create(char *cmd, char **line)
{
  job *result = malloc(sizeof(job));
    
  result->exec = (char*)malloc(strlen(cmd) * sizeof(char));
  result->args = (char**)malloc(MAX_ARGS * sizeof(char*));
  for (int i = 0; i < MAX_ARGS; i++)
  {
    result->args[i] = (char*)malloc(strlen(line[i]) * sizeof(char));
  }
  
  strcpy(result->exec, cmd);
  for (int i = 0; i < MAX_ARGS; i++)
  {
    strcpy(result->args[i], line[i]);
  }
  result->input = false;
  result->output = false;

  return result;
}

void job_destroy(job *j)
{
  if (strlen(j->file) != 0)
    free(j->file);
  for (int i = 0; i < MAX_ARGS; i++)
  {
    free(j->args[i]);
  }
  free(j->args);
  free(j->exec);
  free(j);
}

void job_setFile(job *j, char *name)
{
  j->file = (char*)malloc(strlen(name) * sizeof(char));
  strcpy(j->file, name);
}

void job_setIn(job *j, bool in)
{
  j->input = in;
}

void job_setOut(job *j, bool out)
{
  j->output = out;
}

#endif 
